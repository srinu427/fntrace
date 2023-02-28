#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <link.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

extern void __cyg_profile_func_enter (void *, void *);
extern void __cyg_profile_func_exit (void *, void *);


extern void __attribute__((constructor)) trace_begin (void);
extern void __attribute__((destructor)) trace_end (void);

char* line_cache[100];
size_t line_cache_size = 0;
int file_init_done = 0;
pthread_mutex_t ml;

unsigned char* getHexPt(pthread_t pt) {
    unsigned char *ptc = (unsigned char*)(void*)(&pt);
    unsigned char* hex_ptc = (unsigned char*) malloc(sizeof(unsigned char) * sizeof(pt) + 3);
    sprintf((char*)hex_ptc, "0x");
    for (size_t i=0; i<sizeof(pt); i++) {
        sprintf((char*)hex_ptc + (2*(i+1)), "%02x", (unsigned)(ptc[i]));
    }
    hex_ptc[sizeof(pt) + 2] = '\0';
    return hex_ptc;
}


void
trace_begin (void)
{
    printf("trace lib init\n");
}

void dump_data(){
    if (file_init_done == 0){
        if (access("trace_dump.txt", F_OK) == 0){
            remove("trace_dump.txt");
        }
        file_init_done = 1;
    }
    FILE* fp;
    fp = fopen("trace_dump.txt", "a+");
    for (int i=0; i<line_cache_size; i++){
        fprintf(fp, "%s", line_cache[i]);
        free(line_cache[i]);
    }
    fclose(fp);
    line_cache_size = 0;
}
 
void
trace_end (void)
{
    pthread_mutex_lock(&ml);
    printf("trace lib exit\n");
    dump_data();
    pthread_mutex_unlock(&ml);
}
 
void
__cyg_profile_func_enter (void *func,  void *caller)
{
    pthread_mutex_lock(&ml);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long ts_ms = (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
    pthread_t tid = pthread_self();
    pid_t pid_val = getpid();
    unsigned char* tname = getHexPt(tid);
    Dl_info a, b;
    struct link_map* link_mapa;
    struct link_map* link_mapb;
    dladdr1((void*)func,&a,(void**)&link_mapa,RTLD_DL_LINKMAP);
    dladdr1((void*)caller,&b,(void**)&link_mapb,RTLD_DL_LINKMAP);
    int line_size = snprintf(NULL, 0, "ts: %lu,tid: %s,pid: %d,f: %p,l: %s,c: %p,l: %s\n", ts_ms, tname, pid_val, func - link_mapa->l_addr, link_mapa->l_name, caller - link_mapb->l_addr, link_mapb->l_name);
    char* line = malloc(line_size + 1);
    sprintf(line, "ts: %lu,tid: %s,pid: %d,f: %p,l: %s,c: %p,l: %s\n", ts_ms, tname, pid_val, func - link_mapa->l_addr, link_mapa->l_name, caller - link_mapb->l_addr, link_mapb->l_name);
    line[line_size] = '\0';
    free(tname);
    if (line_cache_size >= 100){
        dump_data();
    }
    line_cache[line_cache_size] = line;
    line_cache_size++;
    
    pthread_mutex_unlock(&ml);
}
 
void
__cyg_profile_func_exit (void *func, void *caller)
{
    pthread_mutex_lock(&ml);
    dump_data();
    pthread_mutex_unlock(&ml);
}

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <unordered_map>

#include <dlfcn.h>
#include <link.h>

struct fn_details{
    void* func = nullptr;
    void* caller = nullptr;
    fn_details* parent = nullptr;
    std::vector<fn_details*> called_fns;
};

struct fn_call_history{
    std::vector<fn_details> fn_list;
};

extern "C"{
void __cyg_profile_func_enter (void *, void *) __attribute__((no_instrument_function));
void __cyg_profile_func_exit (void *, void *) __attribute__((no_instrument_function));
}

std::unordered_map<std::thread::id, fn_details*> trace_per_thread;
std::mutex global_trace_lock;

void
__attribute__ ((constructor, no_instrument_function))
trace_begin (void)
{
 //fp_trace = fopen("trace.out", "w");
}
 
void
__attribute__ ((destructor, no_instrument_function))
trace_end (void)
{
    for (std::unordered_map<std::thread::id, fn_details*>::iterator it = trace_per_thread.begin(); it != trace_per_thread.end(); it++){
        fn_details* curr_fnd = it->second;
        while (true){
            if (curr_fnd->called_fns.size() > 0){
                curr_fnd = curr_fnd->called_fns.back();
            }
            else{
                printf("fn %p %p\n", curr_fnd->func, curr_fnd->caller);
                curr_fnd = curr_fnd->parent;
                if (curr_fnd == nullptr){
                    return;
                }
                delete curr_fnd->called_fns.back();
                curr_fnd->called_fns.pop_back();
            }
        }
    }
 //if(fp_trace != NULL) {
 //fclose(fp_trace);
 //}
}
 
void
__cyg_profile_func_enter (void *func,  void *caller)
{
    
    global_trace_lock.lock();
    std::thread::id this_id = std::this_thread::get_id();
    
    fn_details* curr_fn_det = nullptr;

    Dl_info a, b;
    struct link_map* link_mapa;
    struct link_map* link_mapb;
    dladdr1((void*)func,&a,(void**)&link_mapa,RTLD_DL_LINKMAP);
    dladdr1((void*)caller,&b,(void**)&link_mapb,RTLD_DL_LINKMAP);
    printf("e %p %p %s\n", func-link_mapa->l_addr, caller-link_mapb->l_addr, link_mapa->l_name);
    printf("fn %p %p\n", func, caller);

    if (trace_per_thread.find(this_id) != trace_per_thread.end()){
        
        curr_fn_det = trace_per_thread[this_id];
    }
    
    fn_details* tempfnd = new fn_details();
    tempfnd->func = func;
    tempfnd->caller = caller;
    tempfnd->parent = curr_fn_det;

    

    if (curr_fn_det != nullptr) {
        curr_fn_det->called_fns.push_back(tempfnd);
    }

    trace_per_thread[this_id] = tempfnd;
    global_trace_lock.unlock();
}
 
void
__cyg_profile_func_exit (void *func, void *caller)
{
    global_trace_lock.lock();
    std::thread::id this_id = std::this_thread::get_id();

    fn_details* curr_fn_det = nullptr;

    if (trace_per_thread.find(this_id) != trace_per_thread.end()){
        curr_fn_det = trace_per_thread[this_id];
    }

    if (curr_fn_det != nullptr){
        trace_per_thread[this_id]=curr_fn_det;
    }
    global_trace_lock.unlock();
}

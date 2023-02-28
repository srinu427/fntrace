#include <iostream>
#include "find_primes.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <thread>

void print_world(){
    std::cout<<"World"<<std::endl;
}

void print_hello(){
    std::cout<<"Hello"<<std::endl;
}


int main(){

    std::thread th1(&print_some_primes);
    std::thread th2(&print_some_primes);

    th1.join();
    th2.join();

    fork();
    print_hello();
    print_world();
    int status;
    char *args[2];

    args[0] = "/bin/ls";
    args[1] = NULL;

    if ( fork() == 0 )
        execv( args[0], args );
    else
        wait( &status );

    void *mylib;
    int eret;

    mylib = dlopen("dlexmp.so", RTLD_LOCAL | RTLD_LAZY);
    void (*tell_hi_dl)();
    *(void**)(&tell_hi_dl) = dlsym(mylib, "tell_hi");
    tell_hi_dl();
    eret = dlclose(mylib);

    return 0;
}

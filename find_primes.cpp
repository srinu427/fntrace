#include "find_primes.hpp"

#include <chrono>
#include <thread>
#include <random>
#include <iostream>

bool is_prime(int n){
    bool out = true;
    int m = n >> 1;
    for(int i = 2; i <= m; i++) 
    {
        if(n % i == 0)
        { 
            out = false; 
            break;
        }
    }
    return out;
}

void add_random_delay(){
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(100,500);
    int dice_roll = distribution(generator);
    std::this_thread::sleep_for(std::chrono::milliseconds(dice_roll));
}

void print_some_primes(){
    int n = 3;
    while(n < 100){
        add_random_delay();
        if (is_prime(n)) std::cout << "prime found: " << n << std::endl;
        n++;
    }
}

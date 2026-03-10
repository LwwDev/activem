#include <iostream>
#include <chrono>
#include <thread>
#include "sampler.h"

int main(){

Sampler sampler;

while(true){

    Metrics m = sampler.sample();
    std::cout << m.toJson() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}



    return 0;
}
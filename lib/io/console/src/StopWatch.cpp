#include <fishnet/StopWatch.h>

using namespace fishnet::util;

StopWatch::StopWatch():task(""){
    this->start = std::chrono::high_resolution_clock::now();
}

StopWatch::StopWatch(const std::string & task):task(task){
    this->start = clock::now();
}

double StopWatch::stop(){
    stopped = true;
    this-> end = clock::now();
    duration = end -start;
    return duration.count();
}

void StopWatch::stopAndPrint(){
    double ms = stop()*1000.0;
    if (task != ""){
        std::cout << "Task: "<< this->task << "\nTime elapsed: "<< ms << "ms" << std::endl;
    }else {
        std::cout << "Time elapsed: " << ms << "ms" << std::endl;
    }
}

void StopWatch::reset(){
    this->start = clock::now();
    stopped = false;
}

StopWatch::~StopWatch(){
    if(not stopped)
        stopAndPrint();
}
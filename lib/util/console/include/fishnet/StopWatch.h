#pragma once
#include <chrono>
#include <iostream>
namespace fishnet::util{
class StopWatch
{
private:
    typedef std::chrono::high_resolution_clock clock;
    std::chrono::time_point<std::chrono::system_clock> start,end;
    std::chrono::duration<double> duration;
    std::string task;
    bool stopped = false;
public:
    StopWatch();
    StopWatch(const std::string & task);
    double stop();
    void stopAndPrint();
    void reset();
    ~StopWatch();
};
}

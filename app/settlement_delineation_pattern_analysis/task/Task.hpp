#pragma once
#include <sstream>
#include <fishnet/StopWatch.h>
#include <ostream>
#include <iostream>

class Task {
private:
    std::stringstream taskDesc;
    fishnet::util::StopWatch stopWatch;
public:
    constexpr static std::string FISHNET_ID_FIELD = "FISHNET_ID";

    virtual void run() = 0;

    virtual ~Task(){
        auto desc = taskDesc.str();
        if(desc.ends_with("\n")){
            std::cout << desc;
        }else {
            std::cout << desc << std::endl;
        }
    }

    Task & operator << (auto && value ) noexcept {
        taskDesc << value;
        return *this;
    }

    Task & writeDescLine(std::string_view line) noexcept {
        taskDesc << line << std::endl;
        return *this;
    }

    Task & writeDesc (auto && value) noexcept  {
        return *this << value;
    }
};
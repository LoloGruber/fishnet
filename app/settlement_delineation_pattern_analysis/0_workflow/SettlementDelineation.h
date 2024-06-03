#pragma once
#include <vector>
#include <filesystem>

#include "Task.hpp"

#include "JobGenerator.hpp"
#include "SettlementDelineationConfig.hpp"

class SettlementDelineation: public Task{
private:
    std::vector<std::filesystem::path> inputFiles;
    SettlementDelineationConfig config;

public:
    


    void run() override {
        if(inputFiles.empty())
            throw std::runtime_error("No input files provided");
        // JobGenerator()

    }
};
#pragma once
#include "TaskConfig.hpp"

struct SettlementDelineationConfig:public TaskConfig{
    std::filesystem::path jobDirectory;
    std::filesystem::path cfgDirectory;
    JobType lastJobType;
};
#include "TaskConfig.hpp"

struct ConnectedComponentsConfig : public MemgraphTaskConfig {
    ConnectedComponentsConfig(const json & configDescription):MemgraphTaskConfig(configDescription){}
};
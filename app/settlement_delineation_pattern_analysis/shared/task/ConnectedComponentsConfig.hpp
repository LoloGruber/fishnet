#include "TaskConfig.hpp"

class ConnectedComponentsConfig : public MemgraphTaskConfig {
public:
    constexpr static const char * CONTRACTION_STEM_KEY = "contraction-output-stem";
    constexpr static const char * ANALYSIS_STEM_KEY = "analysis-output-stem";

    std::string contractionOutputStem;
    std::string analysisOutputStem;

    ConnectedComponentsConfig(const json & configDescription):MemgraphTaskConfig(configDescription){
        this->jsonDescription.at(CONTRACTION_STEM_KEY).get_to(contractionOutputStem);
        this->jsonDescription.at(ANALYSIS_STEM_KEY).get_to(analysisOutputStem);
    }
};
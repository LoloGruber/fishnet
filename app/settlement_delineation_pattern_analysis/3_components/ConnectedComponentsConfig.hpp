#include "TaskConfig.hpp"

class ConnectedComponentsConfig : public MemgraphTaskConfig {
public:

    constexpr static const char * CONTRACTION_CFG_KEY = "contraction-config-path";
    constexpr static const char * CONTRACTION_DIRECTORY_KEY = "contraction-output-directory";
    constexpr static const char * CONTRACTION_STEM_KEY = "contraction-output-stem";
    constexpr static const char * ANALYSIS_CFG_KEY = "analysis-config-path";
    constexpr static const char * ANALYSIS_DIRECTORY_KEY = "analysis-output-directory";
    constexpr static const char * ANALYSIS_STEM_KEY = "analysis-output-stem";

    std::filesystem::path contractionCfgPath;
    std::filesystem::path contractionOutputDirectory;
    std::string contractionOutputStem;
    std::filesystem::path analysisConfigPath;
    std::filesystem::path analysisOutputDirectory;
    std::string analysisOutputStem;

    ConnectedComponentsConfig(const json & configDescription):MemgraphTaskConfig(configDescription){
        this->jsonDescription.at(CONTRACTION_CFG_KEY).get_to(contractionCfgPath);
        verifyValidFile(contractionCfgPath);
        this->jsonDescription.at(CONTRACTION_DIRECTORY_KEY).get_to(contractionOutputDirectory);
        verifyValidDirectory(contractionOutputDirectory);
        this->jsonDescription.at(CONTRACTION_STEM_KEY).get_to(contractionOutputStem);
        this->jsonDescription.at(ANALYSIS_CFG_KEY).get_to(analysisConfigPath);
        verifyValidFile(analysisConfigPath);
        this->jsonDescription.at(ANALYSIS_DIRECTORY_KEY).get_to(analysisOutputDirectory);
        verifyValidDirectory(analysisOutputDirectory);
        this->jsonDescription.at(ANALYSIS_STEM_KEY).get_to(analysisOutputStem);
    }
private:
    void verifyValidFile(std::filesystem::path const & path){
        if(not std::filesystem::exists(path) || not std::filesystem::is_regular_file(path) ){
            throw std::runtime_error("File "+path.string()+" does not exist");
        }
    }

    void verifyValidDirectory(std::filesystem::path const & path){
        if(not std::filesystem::exists(path) || not std::filesystem::is_directory(path) ){
            throw std::runtime_error("File "+path.string()+" does not exist");
        }
    }
};
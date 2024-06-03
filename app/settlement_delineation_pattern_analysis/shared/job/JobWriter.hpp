#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
#include <fishnet/Shapefile.hpp>
#include "Job.hpp"


using json = nlohmann::json;

class JobWriter{
private:
    static json asFile(const std::filesystem::path  & path) noexcept {
        json output;
        output["class"] = "File";
        output["path"]= path.string();
        if(path.string().ends_with(".shp")){
            std::vector<json> secondaryFiles;
            fishnet::Shapefile shpFile {path};
            for(auto const& path : shpFile.getAssociatedFiles()| std::views::filter([](const auto & path){return path.extension().string() !=".shp";})){
                json secondaryFile;
                secondaryFile["class"]="File";
                secondaryFile["path"]=path.string();
                secondaryFiles.push_back(std::move(secondaryFile));
            }
            output["secondaryFiles"]=secondaryFiles;
        }
        return output;
    }

    static void writeJson(const json & jsonJob, const std::filesystem::path & path){
        std::ofstream os {path.string()};
        os << std::setw(4) << jsonJob << std::endl;
    }

    static void writeJson(const json & jsonJob, const Job & job){
        std::ofstream os {job.file.string()};
        os << std::setw(4) << jsonJob << std::endl;
    }


public:
    static void write(const FilterJob & filterJob){
        json output;
        output["shpFile"]= asFile(filterJob.input);
        output["config"] = asFile(filterJob.config);
        writeJson(output,filterJob.file);
    }

    static void write(const NeighboursJob & neighboursJob){
        json output;
        std::vector<json> inputFiles;
        for(const auto & input: neighboursJob.additionalInput){
            inputFiles.push_back(asFile(input));
        }
        output["primaryInput"] = neighboursJob.primaryInput;
        output["additionalInput"] = inputFiles;
        output["config"] = asFile(neighboursJob.config);
        output["taskID"] = neighboursJob.id;
        writeJson(output,neighboursJob.file);
    }

    static void write(const ComponentsJob & componentsJob){
        json output;
        output["config"] = componentsJob.config;
        writeJson(output,componentsJob);
    }

    static void write(const ContractionJob & contractionJob){
        json output;
        std::vector<json> inputFiles;
        for(const auto & input: contractionJob.inputs){
            inputFiles.push_back(asFile(input));
        }
        output["shpFiles"] = inputFiles;
        output["config"] = asFile(contractionJob.config);
        output["taskID"] = contractionJob.id;
        output["outputStem"] = contractionJob.outputStem;
        writeJson(output,contractionJob.file);
    }

   static void write(const AnalysisJob & analysisJob) {
        json output;
        output["shpFile"]=asFile(analysisJob.input);
        output["config"] = asFile(analysisJob.config);
        output["outputStem"] = analysisJob.outputStem;
        writeJson(output,analysisJob);
    }
};
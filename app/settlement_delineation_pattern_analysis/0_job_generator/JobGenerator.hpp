#pragma once
#include <filesystem>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/PathHelper.h>
#include <fishnet/GISFactory.hpp>
#include "Job.hpp"
#include "JobDAG.hpp"
#include "JobWriter.hpp"
#include "JobGeneratorConfig.hpp"

class JobGenerator{
private:
    JobGeneratorConfig config;
    std::filesystem::path workingDirectory;
    size_t jobIdCounter = 0;
private:
    std::unordered_map<std::filesystem::path,FilterJob> generateWSFSplitFilterJobs(const std::vector<std::filesystem::path> & inputs,JobDAG_t & jobDag)  {
        assert(config.splits > 0);
        if(inputs.size() > 1)
            assert(config.neighbouringFilesPredicateType==NeighbouringFilesPredicateType::WSF);
        std::unordered_map<std::filesystem::path,fishnet::geometry::Vec2D<int>> fileToCoordinate;
        NeighbouringWSFFilesPredicate neighbouringPredicate;
        auto botLeftOpt = neighbouringPredicate.spatialCoordinatesFromFilename(inputs.front().string());
        if(not botLeftOpt && inputs.size() > 1)
            throw std::runtime_error("File not in the correct format\nExpecting: <WSF-Filename>_<longitude>_<latitude>.tif");
        fishnet::geometry::Vec2D<int> botLeft = botLeftOpt.value_or(fishnet::geometry::Vec2D<int>(0,0));
        /*Find bottom left file in the WSF files */
        for(const auto & input: inputs){
            auto coord = neighbouringPredicate.spatialCoordinatesFromFilename(input.string()).value();
            fileToCoordinate.try_emplace(input,coord);
            if(coord.x < botLeft.x || coord.y < botLeft.y)
                botLeft = coord;
        }
        std::unordered_map<std::filesystem::path,FilterJob> result;
        for(const auto & [file,coordinate]: fileToCoordinate){
            SplitJob job;
            job.id = jobIdCounter++;
            auto jobFilename = "Split_"+file.stem().string()+".json";
            job.file = config.jobDirectory / std::filesystem::path(jobFilename);
            job.state = JobState::RUNNABLE;
            job.input = file;
            job.outDir = workingDirectory;
            job.xOffset = (coordinate.x - botLeft.x) * config.splits;
            job.yOffset = (coordinate.y - botLeft.y) * config.splits;
            job.splits = config.splits;
            JobWriter::write(job);
            std::vector<std::filesystem::path> splitFiles;
            for(uint32_t y = 0; y <= config.splits ; y++){
                for(uint32_t x = 0; x <= config.splits; x++){
                    auto splitFilename = fishnet::util::PathHelper::appendToFilename(file,"_"+ std::to_string(x+job.xOffset)+"_"+std::to_string(y+job.yOffset)).replace_extension(".shp").filename();
                    splitFiles.push_back(workingDirectory / splitFilename);
                }
            }
            for(auto && [splitFile,filterJob] : generateFilterJobs(splitFiles,jobDag)){
                jobDag.addEdge(job,filterJob);
                result.try_emplace(splitFile,filterJob);
            }
        }
        config.neighbouringFilesPredicate = NeighbouringFileTilesPredicate();
        return result;
    }

    std::unordered_map<std::filesystem::path,FilterJob> generateFilterJobs(const std::vector<std::filesystem::path> & inputs,JobDAG_t & jobDag) noexcept{
        std::unordered_map<std::filesystem::path,FilterJob> result;
        for(const auto & input : inputs){
            FilterJob job;
            job.id = jobIdCounter++;
            auto jobFilename = "Filter_"+input.stem().string()+".json";
            job.file = config.jobDirectory / std::filesystem::path(jobFilename);
            job.state = JobState::RUNNABLE;
            job.type = JobType::FILTER;
            job.input = input;
            job.config = config.cfgDirectory / std::filesystem::path("filter.json");
            JobWriter::write(job);
            jobDag.addNode(job);
            result.try_emplace(input,std::move(job));
        }
        return result;
    }

    std::vector<NeighboursJob> generateNeighboursJobs(const std::unordered_map<std::filesystem::path,FilterJob> & inputToFilterJobMap,JobDAG_t & jobDag) noexcept {
        auto filteredFilenameMapper = [this](std::filesystem::path const & path){
            return this->workingDirectory / fishnet::util::PathHelper::appendToFilename(path,"_filtered").filename().replace_extension(".shp");
        };
        std::unordered_map<std::filesystem::path,std::unordered_set<std::filesystem::path>> dependencyMap;
        for(const auto & [lhsInput,_] : inputToFilterJobMap){
            dependencyMap.try_emplace(lhsInput,std::unordered_set<std::filesystem::path>());
            for(const auto & [rhsInput,_] : inputToFilterJobMap){
                if(lhsInput == rhsInput)
                    continue;
                if( config.neighbouringFilesPredicate(lhsInput,rhsInput))
                    dependencyMap.at(lhsInput).insert(rhsInput);
            }
        }
        std::unordered_map<std::filesystem::path,NeighboursJob> inputToNeighboursJobMap;
        for(const auto & [input,filterJob]: inputToFilterJobMap){
            NeighboursJob job;
            job.id = jobIdCounter++;
            auto jobFilename = "Neighbour_"+input.stem().string()+".json";
            job.file = config.jobDirectory / std::filesystem::path(jobFilename);
            job.state = JobState::RUNNABLE;
            job.type = JobType::NEIGHBOURS;
            job.config = config.cfgDirectory / std::filesystem::path("neighbours.json");
            job.primaryInput = filteredFilenameMapper(input);

            for(const auto & neighbour: dependencyMap.at(input)){
                jobDag.addEdge(inputToFilterJobMap.at(neighbour),job);
                job.additionalInput.push_back(filteredFilenameMapper(neighbour));
            }
            JobWriter::write(job);
            jobDag.addEdge(filterJob,job);
            inputToNeighboursJobMap.try_emplace(input,std::move(job));
        }

        // for(const auto & [primaryInput,surroundingInputs]:dependencyMap){
        //     for(const auto & additionalInput : surroundingInputs){
        //         jobDag.addEdge(inputToNeighboursJobMap.at(primaryInput),inputToNeighboursJobMap.at(additionalInput));
        //     }
        // }
        std::vector<NeighboursJob> output;
        for(auto && [_,job]: inputToNeighboursJobMap){
            output.push_back(std::move(job));
        }
        return output;
    }

    void generateComponentsJob(const std::vector<NeighboursJob> & predecessors, JobDAG_t & jobDag) noexcept {
        ComponentsJob job;
        job.id = jobIdCounter++;
        auto jobFileName = "ComponentsJob.json";
        job.file = config.jobDirectory / std::filesystem::path(jobFileName);
        job.state = JobState::RUNNABLE;
        job.type = JobType::COMPONENTS;
        job.config = config.cfgDirectory / std::filesystem::path("components.json");
        job.nextJobId = jobIdCounter;
        job.cfgDirectory = config.cfgDirectory;
        job.jobDirectory = config.jobDirectory;
        for(const auto & predecessor: predecessors){
            jobDag.addEdge(predecessor,job);
        }
        JobWriter::write(job);
    }
public:
    JobGenerator(JobGeneratorConfig && config, std::filesystem::path workingDirectory):config(std::move(config)),workingDirectory(std::move(workingDirectory)){}

    void cleanup(JobDAG_t & jobDag){
        for(const auto & entry: std::filesystem::directory_iterator(config.jobDirectory)){
            if(entry.is_regular_file()&& entry.path().extension() == ".json")
                std::filesystem::remove(entry);
        }
        Query("MATCH (n) DETACH DELETE n;").executeAndDiscard(jobDag.getAdjacencyContainer().getConnection());
    }

    void generate(const std::vector<std::filesystem::path> & inputs,JobDAG_t & jobDag) noexcept{
        if(config.cleanup)
            cleanup(jobDag);
        std::unordered_map<std::filesystem::path,FilterJob> filterJobs;
        std::vector<NeighboursJob> neighboursJobs;
        if(config.lastJobType >= JobType::FILTER) {
            if(config.neighbouringFilesPredicateType == NeighbouringFilesPredicateType::WSF || (inputs.size() == 1 && config.splits > 0))
                filterJobs = generateWSFSplitFilterJobs(inputs,jobDag);
            else    
                filterJobs = generateFilterJobs(inputs,jobDag);
        }
        if(config.lastJobType >= JobType::NEIGHBOURS )
            neighboursJobs = generateNeighboursJobs(filterJobs,jobDag);
        if(config.lastJobType >= JobType::COMPONENTS){
            generateComponentsJob(neighboursJobs,jobDag);
        }
    }
};
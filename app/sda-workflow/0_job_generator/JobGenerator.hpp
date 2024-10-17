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
    std::filesystem::path cfgFile;
    size_t jobIdCounter = 0;
private:
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
            job.config = cfgFile;
            JobWriter::write(job);
            jobDag.addNode(job);
            result.try_emplace(input,std::move(job));
        }
        return result;
    }

    std::unordered_map<std::filesystem::path,fishnet::geometry::Vec2D<int>> coordinateForFiles(const std::vector<std::filesystem::path> & inputs){
        std::unordered_map<std::filesystem::path,fishnet::geometry::Vec2D<int>> fileToCoordinate;
        if(inputs.size() == 1){
            fileToCoordinate.try_emplace(inputs.front(),fishnet::geometry::Vec2D<int>(0,0));
            return fileToCoordinate;
        }
        assert(config.neighbouringFilesPredicateType==NeighbouringFilesPredicateType::WSF);
        NeighbouringWSFFilesPredicate neighbouringPredicate;
        for(const auto & input: inputs){
            auto coord = neighbouringPredicate.spatialCoordinatesFromFilename(input.string());
            if(not coord)
                throw std::runtime_error("File not in the correct format\nExpecting: <WSF-Filename>_<longitude>_<latitude>.tif");
            fileToCoordinate.try_emplace(input,coord.value());

        }
        return fileToCoordinate;
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
            job.config = cfgFile;
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
        job.config = cfgFile;
        job.nextJobId = jobIdCounter;
        job.jobDirectory = config.jobDirectory;
        for(const auto & predecessor: predecessors){
            jobDag.addEdge(predecessor,job);
        }
        JobWriter::write(job);
    }
public:
    JobGenerator(JobGeneratorConfig && config, std::filesystem::path workingDirectory, std::filesystem::path cfgFile)
        :config(std::move(config)),workingDirectory(std::move(workingDirectory)){
            if(std::filesystem::is_symlink(cfgFile))
                cfgFile = std::filesystem::read_symlink(cfgFile);
            this->cfgFile = std::filesystem::absolute(cfgFile);
        }

    void cleanup(JobDAG_t & jobDag){
        for(const auto & entry: std::filesystem::directory_iterator(config.jobDirectory)){
            if(entry.is_regular_file()&& entry.path().extension() == ".json")
                std::filesystem::remove(entry);
        }
        Query("MATCH (n) DETACH DELETE n;").executeAndDiscard(jobDag.getAdjacencyContainer().getConnection());
    }

    void generateWSFSplitJobs(const std::vector<std::filesystem::path> & inputs,const std::filesystem::path & outDir,JobDAG_t & jobDag)  {
        assert(config.splits > 0);
        auto fileToCoordinate = coordinateForFiles(inputs);
        fishnet::geometry::Vec2D<int> botLeft {std::numeric_limits<int>::max(),std::numeric_limits<int>::max()};
        /*Find bottom left file in the WSF files */
        for(const auto & [_,coord]:fileToCoordinate){
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
            job.outDir = outDir;
            job.xOffset = (coordinate.x - botLeft.x)/2 * (config.splits+1);
            job.yOffset = (coordinate.y - botLeft.y)/2 * (config.splits+1);
            job.splits = config.splits;
            JobWriter::write(job);
            jobDag.addNode(job);
        }
        config.neighbouringFilesPredicate = NeighbouringFileTilesPredicate();
        config.neighbouringFilesPredicateType = NeighbouringFilesPredicateType::TILES;
    }

    void generate(const std::vector<std::filesystem::path> & inputs,JobDAG_t & jobDag) noexcept{
        if(config.cleanup)
            cleanup(jobDag);
        std::unordered_map<std::filesystem::path,FilterJob> filterJobs;
        std::vector<NeighboursJob> neighboursJobs;
        if(config.lastJobType >= JobType::FILTER) {  
            filterJobs = generateFilterJobs(inputs,jobDag);
        }
        if(config.lastJobType >= JobType::NEIGHBOURS )
            neighboursJobs = generateNeighboursJobs(filterJobs,jobDag);
        if(config.lastJobType >= JobType::COMPONENTS){
            generateComponentsJob(neighboursJobs,jobDag);
        }
    }
};
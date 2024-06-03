#pragma once
#include <filesystem>
#include <fishnet/FunctionalConcepts.hpp>
#include "Job.hpp"
#include "JobDAG.hpp"
#include "JobWriter.hpp"

class JobGenerator{
private:
    fishnet::util::BiPredicate_t<std::filesystem::path> neighbouringFilesPredicate;
    std::filesystem::path jobDirectory;
    std::filesystem::path cfgDirectory;
    JobType lastJobType;
    size_t jobIdCounter = 0;

private:
    std::unordered_map<std::filesystem::path,FilterJob> generateFilterJobs(const std::vector<std::filesystem::path> & inputs,JobDAG_t & jobDag) noexcept{
        std::unordered_map<std::filesystem::path,FilterJob> result;
        for(const auto & input : inputs){
            FilterJob job;
            job.id = jobIdCounter++;
            auto jobFilename = "Filter_"+input.stem().string()+".json";
            job.file = jobDirectory / std::filesystem::path(jobFilename);
            job.state = JobState::RUNNABLE;
            job.type = JobType::FILTER;
            job.input = input;
            job.config = cfgDirectory / std::filesystem::path("filter.json");
            JobWriter::write(job);
            jobDag.addNode(job);
            result.try_emplace(input,std::move(job));
        }
        return result;
    }

    std::vector<NeighboursJob> generateNeighboursJobs(const std::unordered_map<std::filesystem::path,FilterJob> & inputToFilterJobMap,JobDAG_t & jobDag) noexcept {
        auto filteredFilenameMapper = [](std::filesystem::path const & path){
            return fishnet::Shapefile(path).appendToFilename("_filtered").getPath();
        };
        std::unordered_map<std::filesystem::path,std::unordered_set<std::filesystem::path>> dependencyMap;
        for(const auto & [lhsInput,_] : inputToFilterJobMap){
            dependencyMap.try_emplace(lhsInput,std::unordered_set<std::filesystem::path>());
            for(const auto & [rhsInput,_] : inputToFilterJobMap){
                if(lhsInput == rhsInput)
                    continue;
                if( neighbouringFilesPredicate(lhsInput,rhsInput))
                    dependencyMap.at(lhsInput).insert(rhsInput);
            }
        }
        std::unordered_map<std::filesystem::path,NeighboursJob> inputToNeighboursJobMap;
        for(const auto & [input,filterJob]: inputToFilterJobMap){
            NeighboursJob job;
            job.id = jobIdCounter++;
            auto jobFilename = "Neighbour_"+input.stem().string()+".json";
            job.file = jobDirectory / std::filesystem::path(jobFilename);
            job.state = JobState::RUNNABLE;
            job.type = JobType::NEIGHBOURS;
            job.config = cfgDirectory / std::filesystem::path("neighbours.json");
            job.primaryInput = filteredFilenameMapper(input);

            for(const auto & neighbour: dependencyMap.at(input)){
                jobDag.addEdge(inputToFilterJobMap.at(neighbour),job);
                job.additionalInput.push_back(filteredFilenameMapper(neighbour));
            }
            JobWriter::write(job);
            jobDag.addEdge(filterJob,job);
            inputToNeighboursJobMap.try_emplace(input,std::move(job));
        }

        for(const auto & [primaryInput,surroundingInputs]:dependencyMap){
            for(const auto & additionalInput : surroundingInputs){
                jobDag.addEdge(inputToNeighboursJobMap.at(primaryInput),inputToNeighboursJobMap.at(additionalInput));
            }
        }
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
        job.file = jobDirectory / std::filesystem::path(jobFileName);
        job.state = JobState::RUNNABLE;
        job.type = JobType::COMPONENTS;
        job.config = cfgDirectory / std::filesystem::path("components.json");
        job.nextJobId = jobIdCounter;
        job.cfgDirectory = cfgDirectory;
        job.jobDirectory = jobDirectory;
        for(const auto & predecessor: predecessors){
            jobDag.addEdge(predecessor,job);
        }
        JobWriter::write(job);
    }




public:
    JobGenerator(fishnet::util::BiPredicate<std::filesystem::path> auto && neighbouringFilesPredicate,std::filesystem::path jobDirectory,std::filesystem::path cfgDirectory, JobType lastJobTypeToGenerate = JobType::ANALYSIS )
    :neighbouringFilesPredicate(neighbouringFilesPredicate),jobDirectory(std::move(jobDirectory)),cfgDirectory(std::move(cfgDirectory)),lastJobType(lastJobTypeToGenerate){}

    void generate(const std::vector<std::filesystem::path> & inputs,JobDAG_t & jobDag) noexcept{
        std::unordered_map<std::filesystem::path,FilterJob> filterJobs;
        std::vector<NeighboursJob> neighboursJobs;
        if(lastJobType >= JobType::FILTER)
            filterJobs = generateFilterJobs(inputs,jobDag);
        if(lastJobType >= JobType::NEIGHBOURS )
            neighboursJobs = generateNeighboursJobs(filterJobs,jobDag);
        if(lastJobType >= JobType::COMPONENTS){
            generateComponentsJob(neighboursJobs,jobDag);
        }
    }
};
#include <gtest/gtest.h>
#include "Testutil.h"
#include "JobAdjacency.hpp"
#include <fishnet/Graph.hpp>

using namespace fishnet::graph;
using namespace testutil;

using DAGType = decltype(GraphFactory::DAG<Job>(JobAdjacency(MemgraphConnection::create("host",0).value())));

class JobAdjacencyTest: public ::testing::Test{
protected:
    void SetUp() override {
        jobAdj = std::move(MemgraphConnection::create(hostname,port).transform([](auto && connection){return JobAdjacency(std::move(connection));}));
        if(not jobAdj){
            throw std::runtime_error(jobAdj.error());
        }
        jobAdj->addNode(filterJob);
        jobAdj->addNode(secondFilterJob);
        jobAdj->addAdjacency(filterJob,neighboursJob);
        jobAdj->addAdjacency(secondFilterJob,neighboursJob);
        jobAdj->addNode(contractionJob);
        assert(jobAdj->nodes().size()==4);
        assert(jobAdj->getAdjacencyPairs().size()==2);
    }

    void TearDown() {
        if(jobAdj){
            jobAdj->clear();
        }
    }
    u_int16_t port = 7687;
    std::string hostname = "localhost";
    std::expected<JobAdjacency,std::string> jobAdj = std::unexpected("Not initialized yet"); 
    Job filterJob {1,"filter.json",JobType::FILTER,JobState::RUNNABLE};
    Job secondFilterJob {2,"otherFilter.json",JobType::FILTER,JobState::RUNNABLE};
    Job neighboursJob {3,"neighbours.json",JobType::NEIGHBOURS,JobState::RUNNABLE};
    Job contractionJob{4,"contraction.json",JobType::CONTRACTION,JobState::RUNNABLE};
    Job notInDatabase {987654321,"notfound.json",JobType::UNDEFINED,JobState::UNDEFINED};
};

TEST_F(JobAdjacencyTest, initDAG){
    EXPECT_NO_FATAL_FAILURE(auto g= GraphFactory::DAG<Job>(JobAdjacency(MemgraphConnection::create(hostname,port).value())));
}

TEST_F(JobAdjacencyTest, addNode){
    EXPECT_TRUE(jobAdj->addNode(Job(5,"test.json",JobType::CONTRACTION,JobState::RUNNABLE)));
    EXPECT_SIZE(jobAdj->nodes(),5);
    Job containedById;
    containedById.id = 5;
    EXPECT_TRUE(jobAdj->contains(containedById));
    EXPECT_TRUE(jobAdj->addNode(filterJob));
    EXPECT_SIZE(jobAdj->nodes(),5);
}

TEST_F(JobAdjacencyTest, addNodes){
    std::vector<Job> jobs = {
        {100,"test",JobType::ANALYSIS,JobState::FAILED},
        {101,"test2",JobType::ANALYSIS,JobState::FAILED}
    };
    EXPECT_TRUE(jobAdj->addNodes(jobs));
    EXPECT_SIZE(jobAdj->nodes(),6);
    EXPECT_CONTAINS_ALL(jobAdj->nodes(),jobs);
}

TEST_F(JobAdjacencyTest, addAdjacency){
    EXPECT_SIZE(jobAdj->getAdjacencyPairs(),2);
    EXPECT_FALSE(jobAdj->hasAdjacency(neighboursJob,contractionJob));
    EXPECT_TRUE(jobAdj->addAdjacency(neighboursJob,contractionJob));
    EXPECT_SIZE(jobAdj->getAdjacencyPairs(),3);
    EXPECT_TRUE(jobAdj->hasAdjacency(neighboursJob,contractionJob));
}

TEST_F(JobAdjacencyTest, addAdjacencies){
    std::vector<Job> jobs = {
        {100,"test",JobType::ANALYSIS,JobState::FAILED},
        {101,"test2",JobType::ANALYSIS,JobState::FAILED}
    };
    std::vector<std::pair<Job,Job>> adjJobs = {
        {contractionJob,jobs[0]},{jobs[0],jobs[1]}
    };
    EXPECT_TRUE(jobAdj->addAdjacencies(adjJobs));
    EXPECT_TRUE(jobAdj->hasAdjacency(contractionJob,jobs[0]));
    EXPECT_TRUE(jobAdj->hasAdjacency(jobs[0],Job(jobs[1].id,"",JobType::ANALYSIS,JobState::FAILED)));
    EXPECT_SIZE(jobAdj->getAdjacencyPairs(),4);
}

TEST_F(JobAdjacencyTest, removeNode){
    EXPECT_TRUE(jobAdj->removeNode(neighboursJob));
    EXPECT_FALSE(jobAdj->contains(neighboursJob));
    EXPECT_SIZE(jobAdj->nodes(),3);
    EXPECT_EMPTY(jobAdj->getAdjacencyPairs());
}

TEST_F(JobAdjacencyTest, removeAdjacency) {
    EXPECT_TRUE(jobAdj->hasAdjacency(filterJob,neighboursJob));
    EXPECT_TRUE(jobAdj->removeAdjacency(filterJob,neighboursJob));
    EXPECT_FALSE(jobAdj->hasAdjacency(filterJob,neighboursJob));
    EXPECT_SIZE(jobAdj->getAdjacencyPairs(),1);
    EXPECT_SIZE(jobAdj->nodes(),4);
    EXPECT_EMPTY(jobAdj->adjacency(filterJob));
    EXPECT_NO_FATAL_FAILURE(jobAdj->removeAdjacency(filterJob,contractionJob));
}

TEST_F(JobAdjacencyTest, removeAdjacencies) {
    std::vector<std::pair<Job,Job>> toBeRemoved = {
        {filterJob,neighboursJob},{secondFilterJob,neighboursJob}
    };
    EXPECT_TRUE(jobAdj->removeAdjacencies(toBeRemoved));
    EXPECT_EMPTY(jobAdj->getAdjacencyPairs());
    EXPECT_EMPTY(jobAdj->adjacency(secondFilterJob));
    EXPECT_SIZE(jobAdj->nodes(),4);
}

TEST_F(JobAdjacencyTest, clear) {
    jobAdj->clear();
    EXPECT_EMPTY(jobAdj->nodes());
    EXPECT_EMPTY(jobAdj->getAdjacencyPairs());
}

TEST_F(JobAdjacencyTest, contains){
    EXPECT_TRUE(jobAdj->contains(filterJob));
    EXPECT_TRUE(jobAdj->contains(neighboursJob));
    EXPECT_FALSE(jobAdj->contains(Job(404,"",JobType::ANALYSIS,JobState::RUNNABLE)));
    Job notContained;
    notContained.id = 123456789;
    EXPECT_FALSE(jobAdj->contains(notContained));
}

TEST_F(JobAdjacencyTest, hasAdjacency){
    EXPECT_TRUE(jobAdj->hasAdjacency(filterJob,neighboursJob));
    EXPECT_FALSE(jobAdj->hasAdjacency(filterJob,contractionJob));
    EXPECT_FALSE(jobAdj->hasAdjacency(contractionJob,notInDatabase));
}

TEST_F(JobAdjacencyTest, adjacency) {
    auto adjNodes = jobAdj->adjacency(filterJob);
    EXPECT_SIZE(adjNodes,1);
    EXPECT_CONTAINS(adjNodes,neighboursJob);
    EXPECT_EMPTY(jobAdj->adjacency(notInDatabase));
}

TEST_F(JobAdjacencyTest, nodes) {
    auto allNodes = jobAdj->nodes();
    EXPECT_SIZE(allNodes,4);
    EXPECT_UNSORTED_RANGE_EQ(allNodes,std::vector<Job>{filterJob,secondFilterJob,contractionJob,neighboursJob});
}

TEST_F(JobAdjacencyTest, adjPairs) {
    std::vector<std::pair<Job,Job>> expected = {
        {filterJob,neighboursJob},{secondFilterJob,neighboursJob}
    };
    EXPECT_UNSORTED_RANGE_EQ(jobAdj->getAdjacencyPairs(),expected);
}
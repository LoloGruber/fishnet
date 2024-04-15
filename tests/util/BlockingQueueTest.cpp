#include "Testutil.h"
#include <fishnet/BlockingQueue.hpp>
#include <fishnet/FiniteBlockingQueue.hpp>

static inline int MAX_VALUE= 10000;

using namespace testutil;

class BlockingQueueTest: public ::testing::Test {
protected:
    std::shared_ptr<BlockingQueue<int>> q;
    
    void SetUp() override {
        q = std::make_shared<BlockingQueue<int>>();
    }
};

class FiniteBlockingQueueTest: public ::testing::Test {
protected:
    std::shared_ptr<BlockingQueue<int>> q;

    void SetUp() override {
        q = std::make_shared<FiniteBlockingQueue<int>>(20);
    }
};



struct Producer{
    void operator()(std::shared_ptr<BlockingQueue<int>> q,int first=0,int last=MAX_VALUE){
        int i = first;
        while(i < last) {
            q->put(i);
            i++;
        }
    }
};


struct Consumer{
    void operator()(std::shared_ptr<BlockingQueue<int>> q, std::vector<int> & res){
        while(true) {
            auto value = q->take();
            if (not value) {
                q->putPoisonPill();
                return;
            }
            res.push_back(value.get());
        }    
    }

    void operator()(std::shared_ptr<BlockingQueue<int>> q, std::vector<int> & res, std::mutex & mut){
        while(true) {
            auto value = q->take();
            if (not value) {
                q->putPoisonPill();
                return;
            }
            std::lock_guard<std::mutex> l(mut);
            res.push_back(value.get());
        }    
    }
};

TEST_F(BlockingQueueTest, OneProducerOneConsumer) {
    std::vector<int> actual;
    std::vector<int> expected;
    expected.reserve(MAX_VALUE);
    for(int i=0; i <MAX_VALUE; i++) {
        expected.push_back(i);
    }
    std::thread producer(Producer(),q);
    std::thread consumer(Consumer(),q,std::ref(actual));

    producer.join();
    q->putPoisonPill();
    consumer.join();

    EXPECT_EQ(expected.size(), actual.size());
    EXPECT_EQ(expected,actual);
}

TEST_F(BlockingQueueTest, TwoProducerOneConsumer) {
    std::vector<int> actual;
    std::vector<int> expected;
    expected.reserve(MAX_VALUE);
    for(int i=0; i <MAX_VALUE; i++) {
        expected.push_back(i);
    }
    std::thread producer1(Producer(),q,0,MAX_VALUE/2);
    std::thread producer2(Producer(),q,MAX_VALUE/2);
    std::thread consumer(Consumer(),q,std::ref(actual));

    producer1.join();
    producer2.join();
    q->putPoisonPill();
    consumer.join();

    EXPECT_EQ(expected.size(), actual.size());
    std::sort(actual.begin(),actual.end());
    EXPECT_EQ(expected,actual);
}

TEST_F(BlockingQueueTest, OneProducerTwoConsumer) {
    std::vector<int> res1;
    std::vector<int> res2;
    std::thread producer(Producer(),q);
    std::thread consumer1(Consumer(),q,std::ref(res1));
    std::thread consumer2(Consumer(),q,std::ref(res2));

    producer.join();
    q->putPoisonPill();
    consumer1.join();
    consumer2.join();

    std::vector<int> expected;
    for(int i = 0; i < MAX_VALUE;i++) {
        expected.push_back(i);
    }
    
    std::vector<int> actual;
    for(auto e: res1){
        actual.push_back(e);
    }
    for(auto e: res2) {
        actual.push_back(e);
    }
    EXPECT_EQ(expected.size(),actual.size());
    std::sort(actual.begin(),actual.end());
    EXPECT_EQ(expected,actual);
}

TEST_F(BlockingQueueTest, MultiProducerMultiConsumer) {
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::vector<int> results;
    std::vector<int> v1;
    std::mutex mut;
    int N = 5;
    int dataPerProducer = MAX_VALUE/5;
    for(int i=0; i < N; i++) {
        producers.push_back(std::thread(Producer(),q,i*dataPerProducer,(i+1)*dataPerProducer));
        consumers.push_back(std::thread(Consumer(),q,std::ref(results),std::ref(mut))); 
    }
    std::for_each(producers.begin(),producers.end(),[](auto & t){t.join();});
    q->putPoisonPill();
    std::for_each(consumers.begin(),consumers.end(),[](auto & t){t.join();});

    std::vector<int> actual;
    std::vector<int> expected;

    for(auto & v: results ) {
        
        actual.push_back(v);
        
    }
    for(int i = 0; i < MAX_VALUE; i++) {
        expected.push_back(i);
    }
    
    EXPECT_EQ(expected.size(),actual.size());
    std::sort(actual.begin(),actual.end());
    EXPECT_EQ(expected,actual);

}

TEST_F(FiniteBlockingQueueTest, OneProducerOneConsumer){
    std::vector<int> result;
    std::thread producer(Producer(),q);
    std::thread consumer(Consumer(),q,std::ref(result));

    producer.join();
    q->putPoisonPill();
    consumer.join();
    std::vector<int> expected;
    expected.reserve(MAX_VALUE);
    for(int i = 0; i < MAX_VALUE; i++) {
        expected.push_back(i);
    }

    EXPECT_EQ(expected.size(),result.size());
    EXPECT_EQ(expected,result);
}

TEST_F(FiniteBlockingQueueTest, TwoProducerOneConsumer){
    std::vector<int> actual;
    std::vector<int> expected;
    expected.reserve(MAX_VALUE);
    for(int i=0; i <MAX_VALUE; i++) {
        expected.push_back(i);
    }
    std::thread producer1(Producer(),q,0,MAX_VALUE/2);
    std::thread producer2(Producer(),q,MAX_VALUE/2);
    std::thread consumer(Consumer(),q,std::ref(actual));

    producer1.join();
    producer2.join();
    q->putPoisonPill();
    consumer.join();

    EXPECT_EQ(expected.size(), actual.size());
    std::sort(actual.begin(),actual.end());
    EXPECT_EQ(expected,actual);
}

TEST_F(FiniteBlockingQueueTest, OneProducerTwoConsumer) {
    std::vector<int> res1;
    std::vector<int> res2;
    std::thread producer(Producer(),q);
    std::thread consumer1(Consumer(),q,std::ref(res1));
    std::thread consumer2(Consumer(),q,std::ref(res2));

    producer.join();
    q->putPoisonPill();
    consumer1.join();
    consumer2.join();

    std::vector<int> expected;
    for(int i = 0; i < MAX_VALUE;i++) {
        expected.push_back(i);
    }
    
    std::vector<int> actual;
    for(auto e: res1){
        actual.push_back(e);
    }
    for(auto e: res2) {
        actual.push_back(e);
    }
    EXPECT_EQ(expected.size(),actual.size());
    std::sort(actual.begin(),actual.end());
    EXPECT_EQ(expected,actual);
}

TEST_F(FiniteBlockingQueueTest, MultiProducerMultiConsumer) {
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::vector<int> results;
    std::vector<int> v1;
    std::mutex mut;
    int N = 5;
    int dataPerProducer = MAX_VALUE/5;
    for(int i=0; i < N; i++) {
        producers.push_back(std::thread(Producer(),q,i*dataPerProducer,(i+1)*dataPerProducer));
        consumers.push_back(std::thread(Consumer(),q,std::ref(results),std::ref(mut))); 
    }
    std::for_each(producers.begin(),producers.end(),[](auto & t){t.join();});
    q->putPoisonPill();
    std::for_each(consumers.begin(),consumers.end(),[](auto & t){t.join();});

    std::vector<int> actual;
    std::vector<int> expected;

    for(auto & v: results ) {
        
        actual.push_back(v);
        
    }
    for(int i = 0; i < MAX_VALUE; i++) {
        expected.push_back(i);
    }
    
    EXPECT_EQ(expected.size(),actual.size());
    std::sort(actual.begin(),actual.end());
    EXPECT_EQ(expected,actual);
}


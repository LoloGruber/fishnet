#pragma once
#include <gtest/gtest.h>
#include <stacktrace>
#include <fishnet/CollectionConcepts.hpp>

namespace fishnet::testutil{

static auto message(auto... vals) {
    std::stringstream ss;
    ss << "\033[1;31m";
    ((ss << vals), ...);
    ss << "\033[0m";
    return ss.str();
}

static auto trace(){
    std::stringstream ss;
    ss << "\033[1;36m";
    ss << "Stack trace:" << std::endl;
    auto st = std::stacktrace::current();
    auto it = st.begin();
    while(it != st.end() && it->source_file().ends_with("TestUtil.hxx"))
         ++it;
    --it;
    auto first = it;
    while(not it->source_file().ends_with("gtest.cc") && it != st.end()) {
        if (it != first)
            ss << std::endl;
        ss << "\t" << it->source_file() << ":" << it->source_line();
        ++it;
    }
    ss << "\033[0m";
    return ss.str();
}

static auto message_with_trace(auto... vals) {
    std::stringstream ss;
    ss << message(vals...) << std::endl;
    ss << trace();
    return ss.str();
}

template<typename T>
static testing::AssertionResult contains(const fishnet::util::input_range_of<T> auto & collection, const T & element){
    if(collection.end() != std::ranges::find(collection,element)){
        return testing::AssertionSuccess() << message("Collection contains element: ", element);
    }else{
        return testing::AssertionFailure() << message("Collection does not contain element: ", element);
    }
}

static void TODO() {
   FAIL() << message_with_trace("Test not implemented yet");  
}

template<typename T>
static void EXPECT_CONTAINS(const fishnet::util::input_range_of<T> auto & collection, const T & element) {
    EXPECT_TRUE(contains(collection,element)) << trace();
}

template<typename T>
static void EXPECT_NOT_CONTAINS(const fishnet::util::input_range_of<T> auto & collection, const T & element) {
    EXPECT_FALSE(contains(collection,element)) << trace();
}

}

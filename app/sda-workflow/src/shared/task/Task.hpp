#pragma once
#include <sstream>
#include <fishnet/StopWatch.h>
#include <ostream>
#include <iostream>
#include <expected>
#include <fishnet/Normalize.hpp>
#include <nlohmann/json.hpp>

using json=nlohmann::json;
/**
 * @brief Common super class for all tasks
 * -User can specify a description of the task
 * -Execution time is tracked 
 * -RAII type -> description and execution time is printed on destruction
 */
class Task {
protected:
    json desc;
    fishnet::util::StopWatch stopWatch;
    size_t workflowID = 0;
public:
    Task()=default;
    Task(size_t workflowID):workflowID(workflowID){}

    constexpr static std::string FISHNET_ID_FIELD = "FISHNET_ID";

    virtual void run() = 0;

    virtual ~Task(){
        this->desc["duration[s]"]=stopWatch.stop();
        std::cout <<  this->desc.dump(4) << std::endl;
    }
};


/**
 * @brief Helper functions to deal with std::expected types and unwraps them if possible or throws error
 * @throws runtime_error if not value is present
 * @tparam T value type
 * @tparam E error type
 * @param expected expected containing either a value or an error
 * @return T& reference to the contained value
 */
template<typename T, typename E>
T & getExpectedOrThrowError(std::expected<T,E> & expected) {
    if(not expected){
        throw std::runtime_error(expected.error());
    }
    return expected.value();
}

/**
 * @brief Helper functions to deal with std::expected types and unwraps them if possible or throws error
 * @throws runtime_error if not value is present
 * @tparam T value type
 * @tparam E error type
 * @param expected expected containing either a value or an error
 * @return T  contained value
 */
template<typename T, typename E>
T  getExpectedOrThrowError(std::expected<T,E> && expected) {
    if(not expected){
        throw std::runtime_error(expected.error());
    }
    return std::move(expected.value());
}

/**
 * @brief Helper function to ensure a value is present in the expected (otherwise throws error)
 * @throws runtime_error if not value is present
 * @tparam T value type
 * @tparam E error type
 * @param expected expected containing either a value or an error
 */
template<typename T, typename E>
void testExpectedOrThrowError(std::expected<T,E> & expected) {
    if(not expected){
        throw std::runtime_error(expected.error());
    }
}

uint64_t normalizeToShpFileIntField(std::integral auto number) noexcept {
    const uint64_t SHP_MAX_VALUE = 1000000000000000000ULL;
    return fishnet::math::normalize(number,SHP_MAX_VALUE); 
}

#pragma once
#include <sstream>
#include <fishnet/StopWatch.h>
#include <ostream>
#include <iostream>
#include <expected>

/**
 * @brief Common super class for all tasks
 * -User can specify a description of the task
 * -Execution time is tracked 
 * -RAII type -> description and execution time is printed on destruction
 */
class Task {
private:
    std::stringstream taskDesc;
    fishnet::util::StopWatch stopWatch;
public:
    constexpr static std::string FISHNET_ID_FIELD = "FISHNET_ID";

    virtual void run() = 0;

    virtual ~Task(){
        auto desc = taskDesc.str();
        if(desc.ends_with("\n")){
            std::cout << desc;
        }else {
            std::cout << desc << std::endl;
        }
    }

    Task & operator << (auto && value ) noexcept {
        taskDesc << value;
        return *this;
    }

    Task & writeDescLine(std::string_view line) noexcept {
        taskDesc << line << std::endl;
        return *this;
    }

    Task & writeDesc (auto && value) noexcept  {
        return *this << value;
    }

    Task & indentDesc(auto && value ) noexcept {
        taskDesc << "\t" << value;
        return *this;
    }

    Task & indentDescLine(std::string_view line) noexcept {
        taskDesc << "\t" << line << std::endl;
        return *this;
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
    return expected.value();
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

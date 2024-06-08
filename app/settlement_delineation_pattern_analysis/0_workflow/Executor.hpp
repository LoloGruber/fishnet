#pragma once
#include <fishnet/FunctionalConcepts.hpp>
#include "Job.hpp"

using Callback_t = std::function<void(Job & job)>; // callback function on complete
using Executor_t = std::function<void(Job & job)>; // executor function

template<typename E>
concept Executor= std::convertible_to<E,Executor_t>  && requires(E & executor,Callback_t && cb){
    {executor.setCallback(std::move(cb))};
};

enum class ExecutorType{
    CWLTOIL, CWLTOOL
};
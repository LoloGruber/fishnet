#pragma once
#include "../../math/include/fishnet/NumericConcepts.hpp"
#include "FunctionalConcepts.hpp"

namespace fishnet {
template<typename T>
concept IDProducer= requires(){{ T::operator() } -> math::Number;}
    || requires(T &idProducer){{ idProducer.operator() } -> math::Number;};
}
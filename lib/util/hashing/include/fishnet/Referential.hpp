#pragma once
#include <memory>
#include "HashConcepts.hpp"

namespace fishnet::util{
struct Referential{
    struct Equal
    {   
        template<typename T> requires std::equality_comparable<T>
        bool operator()(std::shared_ptr<T> const & lhs, std::shared_ptr<T> const & rhs) const {
            return *lhs == *rhs;
        }

        template<typename T> requires std::equality_comparable<T>
        bool operator()(std::shared_ptr<const T> const & lhs, std::shared_ptr<const T> const & rhs) const {
            return *lhs == *rhs;
        }

        template<typename T> requires std::equality_comparable<T>
        bool operator()(std::unique_ptr<T> const & lhs, std::unique_ptr<T> const & rhs) const {
            return *lhs==*rhs;
        }
    };

    struct Hash
    {
        template<typename T> requires Hashable<T>
        std::size_t operator()(const std::shared_ptr<T> & p) const{
            return std::hash<T>()(*p);
        }

        template<typename T> requires Hashable<T>
        std::size_t operator()(const std::unique_ptr<T> &p) const{
            return std::hash<T>()(*p);
        }
    };
};
}

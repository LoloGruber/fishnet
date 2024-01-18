#pragma once
#include <stdexcept>
#include <string>

namespace fishnet::geometry {
class InvalidGeometryException : public std::invalid_argument {

public: 
    InvalidGeometryException(std::string const & message): std::invalid_argument(message) {}
    InvalidGeometryException():std::invalid_argument("") {}
};
}
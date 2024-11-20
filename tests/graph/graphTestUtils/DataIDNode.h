#pragma once

#include "IDNode.h"
#include <functional>
#include <string>
#include <fishnet/Printable.hpp>

template<typename T>
class DataIDNode : public IDNode
{
private:
    T data;
public:
    DataIDNode(const T & data):IDNode(), data(data){};
    ~DataIDNode() = default;
    T getData() const {
        return this->data;
    }
    std::string toString() const noexcept requires std::convertible_to<T,std::string>{
        return std::string(data);
    }
};

namespace std{
    template<typename T>
    struct hash<DataIDNode<T>>{
        size_t operator()(const DataIDNode<T> & k) const {
            return (std::size_t) k.getId();
        }
    };
}

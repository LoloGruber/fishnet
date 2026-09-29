#pragma once
#include <fishnet/ObjectConcepts.hpp>

class IDNode
{
private:
    static inline int IDCOUNTER = 0;
    int id;
public:
    IDNode(){
        this->id=IDCOUNTER++;
    };

    virtual ~IDNode()=default;

    int getId()const{
        return this->id;
    }

    bool operator==(const IDNode & other)const {
        return this->id == other.getId();
    }

    size_t hash() const noexcept {
        return (size_t) this->id;
    }

    std::string toString() const {
        return "IDNode(" + std::to_string(this->id) + ")";
    }

};
static_assert(fishnet::util::Object<IDNode>,"IDNode should be an Object");
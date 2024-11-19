#pragma once
#include <mgclient.hpp>
#include "MemgraphConnection.hpp"
#include "CipherQuery.hpp"
#include <magic_enum.hpp>

enum class Label{
    Job,File,Settlement,Component,Undefined, before,stored,neighbours,part_of
};

struct Node{
    static inline size_t sessionID=0;
    std::string_view name = "";
    Label label = Label::Undefined;
    std::string_view attributes = "";

    constexpr friend std::ostream & operator<<(std::ostream & oss, const Node & node) noexcept{
        oss << "(";
        if(not node.name.empty())
            oss << node.name;
        if(node.label != Label::Undefined){
            oss << ":" << magic_enum::enum_name(node.label);
            if(Node::sessionID != 0)
                oss << "_" << Node::sessionID;
        }
        if(not node.attributes.empty())
            oss << "{"<<node.attributes << "}";
        oss << ")";
        return oss;
    }
};

template<bool directed= false>
struct AbstractRelation{
    static inline size_t sessionID=0;
    std::string_view name = "";
    Node from;
    Label label = Label::Undefined;
    Node to;

    constexpr friend std::ostream & operator<<(std::ostream & oss, const AbstractRelation & rel) noexcept{
        oss << rel.from;
        oss << "-[" << rel.name;
        if(rel.label != Label::Undefined){
            oss << ":" << magic_enum::enum_name(rel.label);
            if(Node::sessionID != 0)
                oss << "_" << Node::sessionID;
        }
        oss << "]-";
        if(directed)
            oss << ">" ;
        oss << rel.to;
        return oss;
    }
};

using Relation = AbstractRelation<true>;
using BiRelation = AbstractRelation<false>;

constexpr static inline int64_t asInt(size_t value) {
    return mg::Id::FromUint(value).AsInt();
}

constexpr static inline size_t asNodeIdType(int64_t value) {
    return mg::Id::FromInt(value).AsUint();
} 

constexpr static inline void setSessionID(size_t sessionID){
    assert(sessionID != 0);
    Relation::sessionID = sessionID;
    Relation::sessionID = sessionID;
    Node::sessionID = sessionID;
}

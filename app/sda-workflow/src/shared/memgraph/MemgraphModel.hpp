#pragma once
#include <variant>
#include <mgclient.hpp>
#include <magic_enum.hpp>
#include "MemgraphConnection.hpp"
#include "CipherQuery.hpp"

enum class Label{
    Job,File,Settlement,Component,Undefined, before,stored,neighbours,part_of
};

struct Node{
    std::string_view name = "";
    Label label = Label::Undefined;
    std::string_view attributes = "";

    constexpr friend std::ostream & operator<<(std::ostream & oss, const Node & node) noexcept{
        oss << "(";
        if(not node.name.empty())
            oss << node.name;
        if(node.label != Label::Undefined){
            oss << ":" << magic_enum::enum_name(node.label);
            if(MemgraphConnection::hasSession())
                oss << "_" << MemgraphConnection::getSession().id();
        }
        if(not node.attributes.empty())
            oss << "{"<<node.attributes << "}";
        oss << ")";
        return oss;
    }
};

struct Var{
    std::string_view name;

    constexpr friend std::ostream & operator<<(std::ostream & oss, const Var & variable) noexcept{
        oss << "("<< variable.name << ")";
        return oss;
    }
};

template<bool directed= false>
struct AbstractRelation{
    std::string_view name = "";
    std::variant<Node,Var> from;
    Label label = Label::Undefined;
    std::variant<Node,Var> to;

    constexpr friend std::ostream & operator<<(std::ostream & oss, const AbstractRelation & rel) noexcept{
        std::visit([&oss](auto&& value){oss << value;},rel.from);
        // oss << rel.from;
        oss << "-[" << rel.name;
        if(rel.label != Label::Undefined){
            oss << ":" << magic_enum::enum_name(rel.label);
            if(MemgraphConnection::hasSession())
                oss << "_" << MemgraphConnection::getSession().id();
        }
        oss << "]-";
        if(directed)
            oss << ">" ;
        std::visit([&oss](auto&& value){oss << value;},rel.to);
        // oss << rel.to;
        return oss;
    }
};

struct Index{
    Label label;
    std::string_view fields="";

    constexpr friend std::ostream & operator<<(std::ostream & oss, const Index & index) noexcept{
        assert(index.label != Label::Undefined);
        oss << ":" << magic_enum::enum_name(index.label);
        if(MemgraphConnection::hasSession())
                oss << "_" << MemgraphConnection::getSession().id();
        if(not index.fields.empty())
            oss << "("<< index.fields <<")";
        return oss;
    }
};

using Relation = AbstractRelation<true>;
using BiRelation = AbstractRelation<false>;

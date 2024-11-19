#pragma once
#include <variant>
#include <mgclient.hpp>
#include <magic_enum.hpp>
#include "MemgraphConnection.hpp"
#include "CipherQuery.hpp"

constexpr static inline int64_t asInt(size_t value) {
    return mg::Id::FromUint(value).AsInt();
}

constexpr static inline size_t asNodeIdType(int64_t value) {
    return mg::Id::FromInt(value).AsUint();
} 

/**
 * @brief Stores an unique ID for every memgraph session (e.g. for different concurrent workflow runs)
 * 
 */
class Session{
    friend class std::optional<Session>;
private:
    size_t _id = 0;
    Session(size_t id):_id(id){}
    static inline std::unique_ptr<Session> current = nullptr;
public:
    Session(const Session & session):_id(session.id()){}

    constexpr size_t id() const noexcept {
        return this->_id;
    }

    constexpr static inline Session get() noexcept {
        assert(Session::exists());
        return *Session::current.get();
    }

    constexpr static inline bool exists() noexcept {
        return Session::current != nullptr;
    }

    constexpr static inline void set(const Session & session){
        Session::current = std::unique_ptr<Session>(new Session(session));
    }

    constexpr static inline void reset(){
        Session::current = nullptr;
    }
    
    /**
     * @brief Creates a new session, by obtaining a unique session from the database.
     * Session id is added to all labels in the queries to distinguish different workflow runs
     * 
     */
    static Session createAndSet(const MemgraphConnection & connection) {
        if(CipherQuery("MATCH (s:Session) WITH MAX(s.id) AS maxId ").merge("(n:Session {id: coalesce(maxId,0)+1})").ret("n.id").execute(connection)){
            auto result = connection->FetchAll();
            if(result && result->front().front().type() == mg::Value::Type::Int){
                size_t id = asNodeIdType(result->front().front().ValueInt());
                Session::current = std::unique_ptr<Session>(new Session(id));
                return Session::get();
            }
        }
        throw std::runtime_error("Could not create session");
    }

    /**
     * @brief Checks if a session with that id exists and returns it if present
     * 
     * @param connection db connection
     * @param id id of session
     * @return std::optional<Session> 
     */
    static std::optional<Session> of(const MemgraphConnection & connection, size_t id){
        if(CipherQuery("MATCH (s:Session {id:$sid})").setInt("sid",id).ret("s.id").execute(connection)) {
            auto result = connection->FetchAll();
            if(result && result->front().front().type() == mg::Value::Type::Int){
                size_t id = asNodeIdType(result->front().front().ValueInt());
                return std::optional<Session>(Session(id));
            }
        }
        return std::nullopt;
    }
};

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
            if(Session::exists())
                oss << "_" << Session::get().id();
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
            if(Session::exists())
                oss << "_" << Session::get().id();
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
        if(not index.fields.empty())
            oss << "("<< index.fields <<")";
        return oss;
    }
};

using Relation = AbstractRelation<true>;
using BiRelation = AbstractRelation<false>;

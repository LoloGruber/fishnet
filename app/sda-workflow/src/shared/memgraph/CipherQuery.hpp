#pragma once
#include <sstream>
#include <ranges>
#include <unordered_map>
#include <mgclient.hpp>
#include <iostream>
#include "MemgraphConnection.hpp"

/**
 * @brief Helper class to build cipher queries
 * 
 */
class CipherQuery{
public:
    enum class RelationshipDirection{
        LEFT,RIGHT,NONE
    };

    constexpr static uint8_t MAX_RETRIES = 1;

protected:
    std::unordered_map<std::string, mg::Value> params;
    std::ostringstream query {std::ios::ate};

    constexpr void addVariables(char variable){
        query << variable;
    }

    template<typename... Chars>
    constexpr void addVariables(char variable, Chars... additional){
        query << variable << ",";
        addVariables(additional...);
    }

    constexpr inline void prefixRelationship(RelationshipDirection direction){
        switch(direction){
            case RelationshipDirection::LEFT:
                query << "<-";
                break;
            default:
                query << "-";
        }
    }

    constexpr inline void postfixRelationship(RelationshipDirection direction){
        switch(direction){
            case RelationshipDirection::RIGHT:
                query << "->";
                break;
            default:
                query << "-";
        }
    }

public:
    CipherQuery() = default;

    CipherQuery(CipherQuery && other):params(std::move(other.params)),query(std::move(other.query)) {}

    CipherQuery & operator=(CipherQuery && other)noexcept {
        this->params = std::move(other.params);
        this->query = std::move(other.query);
        return *this;
    }

    constexpr CipherQuery &&  match() && noexcept {
        query << "MATCH ";
        return std::move(*this);
    }

    constexpr CipherQuery &  match() & noexcept {
        query << "MATCH ";
        return *this;
    }

    constexpr CipherQuery && merge() && noexcept{
        query << "MERGE ";
        return std::move(*this);
    }

    constexpr CipherQuery & merge() & noexcept{
        query << "MERGE ";
        return *this;
    }

    constexpr CipherQuery && create() && noexcept{
        query << "CREATE ";
        return std::move(*this);
    }

    constexpr CipherQuery & create() & noexcept{
        query << "CREATE ";
        return *this;
    }

    template<typename... Chars>
    constexpr CipherQuery && ret(char variable, Chars... additional) && noexcept {
        query << "RETURN ";
        addVariables(variable,additional...);
        return std::move(*this);
    }

    template<typename... Chars>
    constexpr CipherQuery & ret(char variable, Chars... additional) & noexcept {
        query << "RETURN ";
        addVariables(variable,additional...);
        return *this;
    }   

    constexpr CipherQuery && node(char variable) && noexcept {
        query << "(" <<variable<<")";
        return std::move(*this);
    }

    constexpr CipherQuery & node(char variable) & noexcept {
        query << "(" <<variable<<")";
        return *this;
    }
    constexpr CipherQuery && node(std::string_view label) && noexcept {
        query << "(:" <<label << ")";
        return std::move(*this);
    }
    constexpr CipherQuery & node(std::string_view label) & noexcept {
        query << "(:" <<label << ")";
        return *this;
    }
    constexpr CipherQuery && node(char variable, std::string_view label) && noexcept {
        query << "(" << variable << ":" <<label << ")";
        return std::move(*this);
    }

    constexpr CipherQuery & node(char variable, std::string_view label) & noexcept {
        query << "(" << variable << ":" <<label << ")";
        return *this;
    }
    constexpr CipherQuery && node(std::string_view label,std::string_view attributes) && noexcept {
        query << "(:" << label << " {" << attributes <<"})" ;
        return std::move(*this);
    }
    constexpr CipherQuery & node(std::string_view label,std::string_view attributes) & noexcept {
        query << "(:" << label << " {" << attributes <<"})" ;
        return *this;
    }
    constexpr CipherQuery && node(char variable, std::string_view label,std::string_view attributes) && noexcept {
        query << "(" << variable << ":" << label << " {" << attributes <<"})" ;
        return std::move(*this);
    }

    constexpr CipherQuery & node(char variable, std::string_view label,std::string_view attributes) & noexcept {
        query << "(" << variable << ":" << label << " {" << attributes <<"})" ;
        return *this;
    }
    
    constexpr CipherQuery && edge(char variable, RelationshipDirection direction = RelationshipDirection::NONE) && noexcept {
        prefixRelationship(direction);
        query << "[" << variable << "]";
        postfixRelationship(direction);
        return std::move(*this);
    }

    constexpr CipherQuery & edge(char variable, RelationshipDirection direction = RelationshipDirection::NONE) & noexcept {
        prefixRelationship(direction);
        query << "[" << variable << "]";
        postfixRelationship(direction);
        return *this;
    }

    constexpr CipherQuery && edge(std::string_view label, char variable=' ',RelationshipDirection direction = RelationshipDirection::NONE) && noexcept {
        prefixRelationship(direction);
        query << "["<< variable << ":"<< label <<"]";
        postfixRelationship(direction);
        return std::move(*this);
    }

    constexpr CipherQuery & edge(std::string_view label, char variable=' ',RelationshipDirection direction = RelationshipDirection::NONE) & noexcept {
        prefixRelationship(direction);
        query << "["<< variable << ":"<< label <<"]";
        postfixRelationship(direction);
        return *this;
    }

    constexpr inline CipherQuery && edge(std::string_view label,RelationshipDirection direction,char variable=' ') && noexcept {
        return std::move(edge(label,variable,direction));
    }

    constexpr inline CipherQuery & edge(std::string_view label,RelationshipDirection direction,char variable=' ') & noexcept {
        return edge(label,variable,direction);
    }

    template<typename T>
    constexpr CipherQuery & line(T && value) & noexcept {
        query << std::forward<T>(value) << std::endl;
        return *this;
    }

    constexpr CipherQuery & endl() & noexcept {
        query << std::endl;
        return *this;
    }

    constexpr CipherQuery && endl() && noexcept {
        query << std::endl;
        return std::move(*this);
    }

    template<typename T>
    constexpr CipherQuery && append(T && value) && noexcept {
        query << std::forward<T>(value);
        return std::move(*this);
    }

    template<typename T>
    constexpr CipherQuery & append(T && value) & noexcept {
        query << std::forward<T>(value);
        return *this;
    }

    constexpr CipherQuery & operator << (auto && value) & noexcept {
        return append(std::forward<decltype(value)>(value));
    }

    constexpr CipherQuery & debug() & noexcept {
        std::cout << query.str() << std::endl;
        return *this;
    }

    constexpr CipherQuery && debug() && noexcept {
        std::cout << query.str() << std::endl;
        return std::move(*this);
    }

    constexpr CipherQuery && set(const std::string_view key, mg::Value && value) && {
        params.try_emplace(std::string(key),value);
        return std::move(*this);
    }

    constexpr CipherQuery & set(const std::string_view key, mg::Value && value) & {
        params.try_emplace(std::string(key),value);
        return *this;
    }

    constexpr CipherQuery && setInt(const std::string_view key, int64_t value) && {
        params.try_emplace(std::string(key),mg::Value(value));
        return std::move(*this);
    }

    constexpr CipherQuery & setInt(const std::string_view key, int64_t value) & {
        params.try_emplace(std::string(key),mg::Value(value));
        return *this;
    }

    constexpr CipherQuery && setInt(const std::string_view key, size_t value) &&{
        params.try_emplace(std::string(key),mg::Value(mg::Id::FromUint(value).AsInt()));
        return std::move(*this);
    }

    constexpr CipherQuery & setInt(const std::string_view key, size_t value) &{
        params.try_emplace(std::string(key),mg::Value(mg::Id::FromUint(value).AsInt()));
        return *this;
    }

    constexpr const auto & getParameters() const noexcept {
        return params;
    }

    constexpr const std::ostringstream & getQuery() const noexcept{
        return query;
    }

    constexpr std::ostringstream & getQuery() noexcept{
        return this->query;
    }

    constexpr std::string asString() const noexcept {
        return this->query.str();
    }

    constexpr bool execute(const MemgraphConnection & connection) {
        mg::Map mgParams {params.size()};
        for(auto && [key,mgValue]:params){
            mgParams.Insert(key,std::move(mgValue));
        }
        bool result = connection->Execute(query.str(),mgParams.AsConstMap());
        int tries = 0;
        while(not result && tries < MAX_RETRIES){
            result = connection.retry()->Execute(query.str(),mgParams.AsConstMap());
        }
        if(not result) {
            std::cerr << "Could not execute query:" << std::endl;
            std::cerr << query.str() << std::endl;
        }
        return result;
    }

    constexpr bool executeAndDiscard(const MemgraphConnection & connection) {
        bool success = execute(connection);
        if(success)
            connection->DiscardAll();
        return success;
    }

    constexpr static CipherQuery DELETE_ALL(){
        return CipherQuery().match().node('n').append("DETACH DELETE ").node('n');
    }
};
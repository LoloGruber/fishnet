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


    constexpr static uint8_t MAX_RETRIES = 1;

protected:
    std::unordered_map<std::string, mg::Value> params;
    std::ostringstream query {std::ios::ate};

    constexpr void addVariables(std::string_view variable){
        query << variable;
    }

    template<typename... Chars>
    constexpr void addVariables(std::string_view variable, Chars... additional){
        query << variable << ",";
        addVariables(additional...);
    }

    template<typename T>
    constexpr void process(std::string_view keyword, T && entity) noexcept {
        this->query << keyword << " " << std::forward<T>(entity)<<" ";
    }

public:
    CipherQuery() = default;

    CipherQuery(std::string_view statement){
        append(statement);
    }

    CipherQuery(CipherQuery && other):params(std::move(other.params)),query(std::move(other.query)) {}

    CipherQuery & operator=(CipherQuery && other)noexcept {
        this->params = std::move(other.params);
        this->query = std::move(other.query);
        return *this;
    }

    template<typename T>
    constexpr CipherQuery &&  match(T && entity) && noexcept {
        process("MATCH",std::forward<T>(entity));
        return std::move(*this);
    }

    template<typename T>
    constexpr CipherQuery &  match(T && entity) & noexcept {
        process("MATCH",std::forward<T>(entity));
        return *this;
    }

    template<typename T>
    constexpr CipherQuery && merge(T && entity) && noexcept{
        process("MERGE",std::forward<T>(entity));
        return std::move(*this);
    }

    template<typename T>
    constexpr CipherQuery & merge(T && entity) & noexcept{
        process("MERGE",std::forward<T>(entity));
        return *this;
    }

    template<typename T>
    constexpr CipherQuery && create(T && entity) && noexcept{
        process("CREATE",std::forward<T>(entity));
        return std::move(*this);
    }

    template<typename T>
    constexpr CipherQuery & create(T && entity) & noexcept{
        process("CREATE",std::forward<T>(entity));
        return *this;
    }

    template<typename T>
    constexpr CipherQuery && where(T && entity) && noexcept{
        process("WHERE",std::forward<T>(entity));
        return std::move(*this);
    }

    template<typename T>
    constexpr CipherQuery & where(T && entity) & noexcept{
        process("WHERE",std::forward<T>(entity));
        return *this;
    }

    template<typename... Chars>
    constexpr CipherQuery && ret(std::string_view variable, Chars... additional) && noexcept {
        query << "RETURN ";
        addVariables(variable,additional...);
        return std::move(*this);
    }

    template<typename... Chars>
    constexpr CipherQuery & ret(std::string_view variable, Chars... additional) & noexcept {
        query << "RETURN ";
        addVariables(variable,additional...);
        return *this;
    }   

    template<typename... Chars>
    constexpr CipherQuery && del(std::string_view variable, Chars... additional) && noexcept {
        query << std::endl <<"DETACH DELETE ";
        addVariables(variable,additional...);
        return std::move(*this);
    }

    template<typename... Chars>
    constexpr CipherQuery & del(std::string_view variable, Chars... additional) & noexcept {
        query << std::endl <<"DETACH DELETE ";
        addVariables(variable,additional...);
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

    constexpr CipherQuery && add(const CipherQuery & other) && noexcept {
        for(const auto & [key,val]:other.params){
            this->params.try_emplace(key,val);
        }
        return std::move(append(other.asString()));
    }

    constexpr CipherQuery & add(const CipherQuery & other) & noexcept {
        for(const auto & [key,val]:other.params){
            this->params.try_emplace(key,val);
        }
        return append(other.asString());
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
            tries++;
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
        return CipherQuery("MATCH (n)").del("n");
    }

    template<typename T>
    constexpr static CipherQuery CREATE_INDEX(T&& index){
        return CipherQuery("CREATE INDEX ON ").append(std::forward<T>(index));
    }

    template<typename T>
    constexpr static CipherQuery CREATE_EDGE_INDEX(T&& index){
        return CipherQuery("CREATE EDGE INDEX ON ").append(std::forward<T>(index));
    }
}; 
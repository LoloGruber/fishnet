#pragma once
#include <sstream>
#include <ranges>
#include <unordered_map>
#include <mgclient.hpp>
#include <iostream>
#include <mgclient.hpp>

/**
 * @brief Helper class to build cipher queries
 * 
 */
class CipherQuery{
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

    CipherQuery(CipherQuery &&) noexcept=default;

    CipherQuery & operator=(CipherQuery && other)noexcept = default;

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

    constexpr static CipherQuery DELETE_ALL(){
        CipherQuery q {"MATCH (n)"};
        q.del("n");
        return q;
    }

    template<typename T>
    constexpr static CipherQuery CREATE_INDEX(T&& index){
        CipherQuery q {"CREATE INDEX ON "};
        q.append(std::forward<T>(index));
        return q;
    }

    template<typename T>
    constexpr static CipherQuery CREATE_EDGE_INDEX(T&& index){
        CipherQuery q {"CREATE EDGE INDEX ON "};
        q.append(std::forward<T>(index));
        return q;
    }

    template<typename T>
    constexpr static CipherQuery DROP_INDEX(T&& index){
        CipherQuery q {"DROP INDEX ON "};
        q.append(std::forward<T>(index));
        return q;
    }

    template<typename T>
    constexpr static CipherQuery DROP_EDGE_INDEX(T&& index){
        CipherQuery q {"DROP EDGE INDEX ON "};
        q.append(std::forward<T>(index));
        return q;
    }

    template<typename T>
    constexpr static CipherQuery MERGE(T && value){
        CipherQuery q;
        q.merge(std::forward<T>(value));
        return q;
    }

    template<typename T>
    constexpr static CipherQuery MATCH(T && value){
        CipherQuery q;
        q.match(std::forward<T>(value));
        return q;
    }

    template<typename T>
    constexpr static CipherQuery CREATE(T && value){
        CipherQuery q;
        q.create(std::forward<T>(value));
        return q;
    }
}; 
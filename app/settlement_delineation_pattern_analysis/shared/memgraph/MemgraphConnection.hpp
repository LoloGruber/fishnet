#pragma once
#include <mgclient.hpp>
#include <memory>
#include <expected>
#include <sstream>
#include <iostream>

class MemgraphConnection {
private:
    std::unique_ptr<mg::Client> connection;
public:
    MemgraphConnection()=default;

    explicit MemgraphConnection(std::unique_ptr<mg::Client> && connection):connection(std::move(connection)){}

    MemgraphConnection(MemgraphConnection && other)noexcept{
        this->connection = std::move(other.connection);
    }

    MemgraphConnection & operator=(MemgraphConnection && other) noexcept {
        this->connection = std::move(other.connection);
        return *this;
    }

    /**
     * @brief Factory Method to create a Memgraph Connection from Memgraph parameters
     * 
     * @param params parameters for the database connection (e.g hostname, port,...)
     * @return std::expected<MemgraphClient,std::string>: Containing the MemgraphClient on success or a string explaining the error
     */
    static std::expected<MemgraphConnection,std::string> create(const mg::Client::Params & params ) {
        auto clientPtr = mg::Client::Connect(params);
        if(not clientPtr){
            std::stringstream connectionError;
            connectionError << "Could not connect to memgraph database!" << std::endl;
            connectionError << "\tHost: " <<params.host << std::endl;
            connectionError << "\tPort: " << std::to_string(params.port) << std::endl;
            connectionError << "\tUsername: " << params.username << std::endl;
            return std::unexpected(connectionError.str());
        }
        return std::expected<MemgraphConnection,std::string>(MemgraphConnection(std::move(clientPtr)));
    }

        /**
     * @brief Factory Method to create a Memgraph client from hostname and port
     * 
     * @param hostname (e.g. localhost)
     * @param port (e.g. 7687)
     * @return std::expected<MemgraphClient,std::string>: Containing the MemgraphClient on success or a string explaining the error
     */
    static std::expected<MemgraphConnection ,std::string> create(std::string hostname, uint16_t port) {
        mg::Client::Params params;
        params.host = hostname;
        params.port = port;
        params.use_ssl = false;
        return create(params);
    }

    const std::unique_ptr<mg::Client> & get() const noexcept {
        return this->connection;
    }

    bool isConnected() const noexcept {
        return this->connection != nullptr;
    }

    mg::Client * operator->()const noexcept {
        return this->connection.get();
    }

    ~MemgraphConnection(){
        connection.reset(nullptr);
        mg::Client::Finalize();
    }
};


/**
 * @brief Helper class for building queries
 * 
 */
class Query {
    protected:
        std::stringstream query;
    public:
        template<typename T>
        Query(T && value){
            append(std::forward<T>(value));
        }

        Query()=default;

        template<typename T>
        Query & append(T && value){
            query << std::forward<T>(value);
            return *this;
        }

        template<typename T>
        Query & line(T && value) {
            query << std::forward<T>(value) << std::endl;
            return *this;
        }

        Query & operator <<(auto && value) {
            return append(value);
        }

        Query & debug() noexcept {
            std::cout << query.str() << std::endl;
            return *this;
        }

        std::stringstream & getQuery() {
            return query;
        }

        bool execute(const MemgraphConnection & connection) {
            return connection->Execute(query.str());
        }

        bool executeAndDiscard(const MemgraphConnection & connection) {
            bool success = connection->Execute(query.str());
            if(success)
                connection->DiscardAll();
            return success;
        }
};

/**
 * @brief Helper class for building parameterized queries
 * 
 */
class ParameterizedQuery{
    private:
        std::unordered_map<std::string, mg::Value> params;
        std::stringstream query;
    public:
        ParameterizedQuery() = default;

        ParameterizedQuery(ParameterizedQuery && other):params(std::move(other.params)),query(std::move(other.query)) {}

        ParameterizedQuery(const ParameterizedQuery & other):params(other.params){
            this->query = std::stringstream(other.query.str());
        }

        ParameterizedQuery(std::string_view value){
            append(std::forward<std::string_view>(value));
        }

        ParameterizedQuery & operator=(ParameterizedQuery && other)noexcept {
            this->params = std::move(other.params);
            this->query = std::move(other.query);
            return *this;
        }

        ParameterizedQuery operator+( ParameterizedQuery const& other)  noexcept{
            ParameterizedQuery result;
            for(const auto & [key,val]:this->params ){
                result.params.try_emplace(key,val);
            }
            for(const auto & [key,val]:other.params){
                result.params.try_emplace(key,val);
            }
            result.line(this->getQuery().str());
            result.line(other.getQuery().str());
            return result;
        }

        template<typename T>
        ParameterizedQuery(int capacity,T && value):params(capacity){
            append(std::forward<T>(value));
        }

        template<typename T> requires(!std::same_as<T,ParameterizedQuery>)
        ParameterizedQuery & append(T && value){
            this->query << std::forward<T>(value);
            return *this;
        }

        template<typename T>
        ParameterizedQuery & line(T && value) {
            this->query << std::forward<T>(value) << std::endl;
            return *this;
        }

        ParameterizedQuery & operator <<(auto && value) {
            return append(value);
        }

        ParameterizedQuery & set(const std::string_view key, mg::Value && value) {
            params.try_emplace(std::string(key),value);
            return *this;
        }

        ParameterizedQuery & setInt(const std::string_view key, int64_t value){
            params.try_emplace(std::string(key),mg::Value(value));
            return *this;
        }

        ParameterizedQuery & setInt(const std::string_view key, size_t value){
            params.try_emplace(std::string(key),mg::Value(mg::Id::FromUint(value).AsInt()));
            return *this;
        }

        const auto & getParameters() const noexcept {
            return params;
        }

        const std::stringstream & getQuery() const {
            return query;
        }

        std::stringstream & getQuery() {
            return this->query;
        }

        ParameterizedQuery & debug() noexcept {
            std::cout << query.str() << std::endl;
            return *this;
        }

        bool execute(const MemgraphConnection & connection) {
            mg::Map mgParams {params.size()};
            for(auto && [key,mgValue]:params){
                mgParams.Insert(key,std::move(mgValue));
                std::cout << key << std::endl;
            }
            return connection->Execute(query.str(),mgParams.AsConstMap());
        }

        bool executeAndDiscard(const MemgraphConnection & connection) {
            bool success = execute(connection);
            if(success)
                connection->DiscardAll();
            return success;
        }
};
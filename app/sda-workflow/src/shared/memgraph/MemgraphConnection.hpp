#pragma once
#include <mgclient.hpp>
#include <memory>
#include <expected>
#include <sstream>
#include <iostream>
#include <fishnet/Either.hpp>

class MemgraphConnection {
private:
    mutable std::unique_ptr<mg::Client> connection;
    mg::Client::Params parameters;
public:
    MemgraphConnection()=default;

    explicit MemgraphConnection(std::unique_ptr<mg::Client> && connection,const mg::Client::Params & params)
    :connection(std::move(connection)),parameters(params){}

    MemgraphConnection(MemgraphConnection && other)noexcept{
        this->connection = std::move(other.connection);
    }

    MemgraphConnection & operator=(MemgraphConnection && other) noexcept {
        this->connection = std::move(other.connection);
        return *this;
    }

    /**
     * @brief Construct a new Memgraph Connection object from existing connection by copying the connection parameters
     * @throws runtime error if connection can not be established
     * @param other 
     */
    MemgraphConnection(const MemgraphConnection & other):MemgraphConnection(std::move(MemgraphConnection::create(other.parameters).value_or_throw())) {}

    const MemgraphConnection & retry() const {
        connection = mg::Client::Connect(this->parameters);
        return *this;
    }
    /**
     * @brief Factory Method to create a Memgraph Connection from Memgraph parameters
     * 
     * @param params parameters for the database connection (e.g hostname, port,...)
     * @return std::expected<MemgraphClient,std::string>: Containing the MemgraphClient on success or a string explaining the error
     */
    static fishnet::util::Either<MemgraphConnection,std::string> create(const mg::Client::Params & params ) {
        auto clientPtr = mg::Client::Connect(params);
        if(not clientPtr){
            std::ostringstream connectionError;
            connectionError << "Could not connect to memgraph database." << std::endl;
            connectionError << "\tHost: " <<params.host << std::endl;
            connectionError << "\tPort: " << std::to_string(params.port) << std::endl;
            return std::unexpected(connectionError.str());
        }
        return fishnet::util::Either<MemgraphConnection,std::string>(MemgraphConnection(std::move(clientPtr),params));
    }

        /**
     * @brief Factory Method to create a Memgraph client from hostname and port
     * 
     * @param hostname (e.g. localhost)
     * @param port (e.g. 7687)
     * @return std::expected<MemgraphClient,std::string>: Containing the MemgraphClient on success or a string explaining the error
     */
    static fishnet::util::Either<MemgraphConnection ,std::string> create(std::string hostname, uint16_t port) {
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


// /**
//  * @brief Helper class for building queries
//  * 
//  */
// class Query {
//     protected:
//         std::ostringstream query;
//     public:
//         template<typename T>
//         Query(T && value){
//             append(std::forward<T>(value));
//         }

//         Query()=default;

//         template<typename T>
//         Query & append(T && value){
//             query << std::forward<T>(value);
//             return *this;
//         }

//         template<typename T>
//         Query & line(T && value) {
//             query << std::forward<T>(value) << std::endl;
//             return *this;
//         }

//         Query & operator <<(auto && value) {
//             return append(value);
//         }

//         Query & debug() noexcept {
//             std::cout << query.str() << std::endl;
//             return *this;
//         }

//         std::ostringstream & getQuery() {
//             return query;
//         }

//         bool execute(const MemgraphConnection & connection) {
//             bool result =  connection->Execute(query.str());
//             if(not result)
//                 result =  connection.retry()->Execute(query.str());
//             return result;
//         }

//         bool executeAndDiscard(const MemgraphConnection & connection) {
//             bool success = connection->Execute(query.str());
//             if(success)
//                 connection->DiscardAll();
//             return success;
//         }
// };

// /**
//  * @brief Helper class for building parameterized queries
//  * 
//  */
// class ParameterizedQuery{
//     private:
//         std::unordered_map<std::string, mg::Value> params;
//         std::ostringstream query {std::ios::ate};
//     public:
//         ParameterizedQuery() = default;

//         ParameterizedQuery(ParameterizedQuery && other):params(std::move(other.params)),query(std::move(other.query)) {
//         }

//         ParameterizedQuery(const ParameterizedQuery & other):params(other.params){
//             this->query = std::ostringstream(std::ios::ate);
//             this->query.str(other.getQuery().str());
//         }

//         ParameterizedQuery(std::string_view value){
//             append(std::forward<std::string_view>(value));
//         }

//         ParameterizedQuery & operator=(ParameterizedQuery && other)noexcept {
//             this->params = std::move(other.params);
//             this->query = std::move(other.query);
//             return *this;
//         }

//         ParameterizedQuery operator+( ParameterizedQuery const& other)  noexcept{
//             ParameterizedQuery result;
//             for(const auto & [key,val]:this->params ){
//                 result.params.try_emplace(key,val);
//             }
//             for(const auto & [key,val]:other.params){
//                 result.params.try_emplace(key,val);
//             }
//             result.line(this->getQuery().str());
//             result.line(other.getQuery().str());
//             return result;
//         }

//         template<typename T>
//         ParameterizedQuery(int capacity,T && value):params(capacity){
//             append(std::forward<T>(value));
//         }

//         template<typename T> requires(!std::same_as<T,ParameterizedQuery>)
//         ParameterizedQuery & append(T && value){
//             this->query << std::forward<T>(value);
//             return *this;
//         }

//         template<typename T>
//         ParameterizedQuery & line(T && value) {
//             this->query << std::forward<T>(value) << std::endl;
//             return *this;
//         }

//         ParameterizedQuery & operator <<(auto && value) {
//             return append(value);
//         }

//         ParameterizedQuery & set(const std::string_view key, mg::Value && value) {
//             params.try_emplace(std::string(key),value);
//             return *this;
//         }

//         ParameterizedQuery & setInt(const std::string_view key, int64_t value){
//             params.try_emplace(std::string(key),mg::Value(value));
//             return *this;
//         }

//         ParameterizedQuery & setInt(const std::string_view key, size_t value){
//             params.try_emplace(std::string(key),mg::Value(mg::Id::FromUint(value).AsInt()));
//             return *this;
//         }

//         const auto & getParameters() const noexcept {
//             return params;
//         }

//         const std::ostringstream & getQuery() const {
//             return query;
//         }

//         std::ostringstream & getQuery() {
//             return this->query;
//         }

//         ParameterizedQuery & debug() noexcept {
//             std::cout << query.str() << std::endl;
//             return *this;
//         }

//         bool execute(const MemgraphConnection & connection) {
//             mg::Map mgParams {params.size()};
//             for(auto && [key,mgValue]:params){
//                 mgParams.Insert(key,std::move(mgValue));
//             }
//             bool result = connection->Execute(query.str(),mgParams.AsConstMap());
//             if(not result){
//                 result = connection.retry()->Execute(query.str(),mgParams.AsConstMap());
//             }
//             if(not result) {
//                 std::cerr << "Could not execute query:" << std::endl;
//                 std::cerr << query.str() << std::endl;
//             }
//             return result;
//         }

//         bool executeAndDiscard(const MemgraphConnection & connection) {
//             bool success = execute(connection);
//             if(success)
//                 connection->DiscardAll();
//             return success;
//         }
// };
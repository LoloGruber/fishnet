#pragma once
#include <mgclient.hpp>
#include <memory>
#include <expected>
#include <sstream>
#include <iostream>
#include <cassert>
#include <fishnet/Either.hpp>
#include <CipherQuery.hpp>

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
public:
    Session()=default;

    constexpr size_t id() const noexcept {
        return this->_id;
    }

    /**
     * @brief Creates a new session, by obtaining a unique session id from the database.
     * 
     */
    static Session makeUnique(const CipherConnection auto & connection) {
        if(CipherQuery("MATCH (s:Session) WITH MAX(s.id) AS maxId ").merge("(n:Session {id: coalesce(maxId,0)+1})").ret("n.id").execute(connection)){
            auto result = connection->FetchAll();
            if(result && result->front().front().type() == mg::Value::Type::Int){
                size_t id = asNodeIdType(result->front().front().ValueInt());
                return Session(id);
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
    static std::optional<Session> of(const CipherConnection auto & connection, size_t id){
        if(CipherQuery("MATCH (s:Session {id:$sid})").setInt("sid",id).ret("s.id").execute(connection)) {
            auto result = connection->FetchAll();
            if(result && not result.value().empty() &&  result->front().front().type() == mg::Value::Type::Int){
                size_t id = asNodeIdType(result->front().front().ValueInt());
                return std::optional<Session>(Session(id));
            }
        }
        return std::nullopt;
    }
};

class MemgraphConnection {
private:
    mutable std::unique_ptr<mg::Client> connection;
    mg::Client::Params params;
     static inline std::unique_ptr<Session> session = nullptr;

    explicit MemgraphConnection(std::unique_ptr<mg::Client> && connection,const mg::Client::Params & params)
    :connection(std::move(connection)),params(params){}

public:
    MemgraphConnection()=default;

    MemgraphConnection(MemgraphConnection && other)noexcept{
        this->connection = std::move(other.connection);
        this->params = std::move(other.params);
    }

    MemgraphConnection & operator=(MemgraphConnection && other) noexcept {
        this->connection = std::move(other.connection);
        this->params = std::move(other.params);
        return *this;
    }

    /**
     * @brief Construct a new Memgraph Connection object from existing connection by copying the connection params
     * @throws runtime error if connection can not be established
     * @param other 
     */
    MemgraphConnection(const MemgraphConnection & other):MemgraphConnection(std::move(MemgraphConnection::create(other.params).value_or_throw())) {}

    const MemgraphConnection & retry() const {
        connection = mg::Client::Connect(this->params);
        return *this;
    }
    /**
     * @brief Factory Method to create a Memgraph Connection from Memgraph params
     * 
     * @param params parameters for the database connection (e.g hostname, port,...)
     * @return Either<MemgraphClient,std::string>: Containing the MemgraphClient on success or a string explaining the error
     */
    static fishnet::util::Either<MemgraphConnection,std::string> create(const mg::Client::Params & params) {
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
     * @brief Factory Method to create a Memgraph Connection from Memgraph params loading a unique Session
     * 
     * @param params parameters for the database connection (e.g hostname, port,...)
     * @param sessionID unique session id to distinguish concurrent workflows runs on the basis of labels
     * @return Either<MemgraphClient,std::string>: Containing the MemgraphClient on success or a string explaining the error
     */
    static fishnet::util::Either<MemgraphConnection,std::string> create(const mg::Client::Params & params, size_t sessionID) {
        auto eitherConnection = create(params);
        if(sessionID==0)
            return eitherConnection; // sessionID of zero implies no session
        if(eitherConnection) {
            auto optSession = Session::of(eitherConnection.value(),sessionID);
            if(optSession) {
                MemgraphConnection::setSession(optSession.value());
            } else{
                return std::unexpected(std::string("Could not load database session with ID: ")+std::to_string(sessionID));
            }
        }
        return eitherConnection;
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


    constexpr static inline Session getSession() noexcept {
        assert(MemgraphConnection::hasSession());
        return *MemgraphConnection::session.get();
    }

    constexpr static inline bool hasSession() noexcept {
        return MemgraphConnection::session != nullptr;
    }

    constexpr static inline void setSession(const Session & session){
        MemgraphConnection::session = std::unique_ptr<Session>(new Session(session));
    }

    constexpr static inline void resetSession(){
        MemgraphConnection::session = nullptr;
    }

    ~MemgraphConnection(){
        connection.reset(nullptr);
        mg::Client::Finalize();
    }
};
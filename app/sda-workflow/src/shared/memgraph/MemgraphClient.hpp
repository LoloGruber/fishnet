#pragma once
#include "MemgraphConnection.hpp"
#include "CipherQuery.hpp"
#include "MemgraphModel.hpp"
#include <unordered_map>
#include <memory>
#include <expected>
#include <sstream>
#include <mgclient.hpp>
#include <filesystem>
#include <fishnet/CollectionConcepts.hpp>


using NodeIdType = size_t;
/**
 * @brief File reference have a unique id for each file
 * 
 */
struct FileReference{
    int64_t fileId;
};

/**
 * @brief Nodes are stored with their unique ID and a reference to the file they are stored in
 * 
 */
struct NodeReference{
    NodeIdType nodeId;
    FileReference fileRef = FileReference(-1);
};


/**
 * @brief Component reference get a unique id on insert, can be used for deletion
 * 
 */
struct ComponentReference{
    int64_t componentId;
};


class MemgraphClient{
private:
    MemgraphConnection mgConnection;
public:
    explicit MemgraphClient(MemgraphConnection && clientPtr):mgConnection(std::move(clientPtr)){
        if(not createConstraints() || not createIndexes()) {
            throw std::runtime_error("Could not create constraints. Check the database connection");
        }
    }


    const std::unique_ptr<mg::Client> & getConnection() const noexcept  {
        return this->mgConnection.get();
    }

    const MemgraphConnection & getMemgraphConnection() const noexcept {
        return this->mgConnection;
    }


    bool createConstraints()const noexcept {
        return mgConnection.executeAndDiscard(
            CipherQuery("CREATE CONSTRAINT ON ").append(Node{.name="n",.label=Label::Settlement}).append(" ASSERT n.id IS UNIQUE"),
            CipherQuery("CREATE CONSTRAINT ON ").append(Node{.name="f",.label=Label::File}).append(" ASSERT f.id IS UNIQUE"),
            CipherQuery("CREATE CONSTRAINT ON ").append(Node{.name="f",.label=Label::File}).append(" ASSERT exists(f.path)"));
    }

    bool dropConstraints() const noexcept {
        return mgConnection.executeAndDiscard(
            CipherQuery("DROP CONSTRAINT ON ").append(Node{.name="n",.label=Label::Settlement}).append(" ASSERT n.id IS UNIQUE"),
            CipherQuery("DROP CONSTRAINT ON ").append(Node{.name="f",.label=Label::File}).append(" ASSERT f.id IS UNIQUE"),
            CipherQuery("DROP CONSTRAINT ON ").append(Node{.name="f",.label=Label::File}).append(" ASSERT exists(f.path)"));
    }

    bool createIndexes() const noexcept {
        return mgConnection.executeAndDiscard(
            CipherQuery::CREATE_INDEX(Index(Label::Settlement,"id")),
            CipherQuery::CREATE_INDEX(Index{.label=Label::File}),
            CipherQuery::CREATE_INDEX(Index{.label=Label::Component}),
            CipherQuery::CREATE_EDGE_INDEX(Index(Label::stored)),
            CipherQuery::CREATE_EDGE_INDEX(Index(Label::neighbours)),
            CipherQuery::CREATE_EDGE_INDEX(Index(Label::part_of)));
    }

    bool dropIndexes() const noexcept {
        return mgConnection.executeAndDiscard(
            CipherQuery::DROP_INDEX(Index(Label::Settlement,"id")),
            CipherQuery::DROP_INDEX(Index{.label=Label::File}),
            CipherQuery::DROP_INDEX(Index{.label=Label::Component}),
            CipherQuery::DROP_EDGE_INDEX(Index(Label::stored)),
            CipherQuery::DROP_EDGE_INDEX(Index(Label::neighbours)),
            CipherQuery::DROP_EDGE_INDEX(Index(Label::part_of)));
    }

    /**
     * @brief Adds a file reference to the database.
     * The node in the database stores the path to the file and has an unique ID
     * @param path path to file
     * @return std::optional<FileReference> containing the unique ID of the file if successful 
     */
    std::optional<FileReference> addFileReference(const std::filesystem::path & pathToFile) const noexcept{
        std::filesystem::path path = pathToFile;
        if(std::filesystem::is_symlink(pathToFile)){
            path = std::filesystem::read_symlink(pathToFile);
        }
        CipherQuery query;
        query.merge(Node("f",Label::File,"path:$path")).set("path",mg::Value(path.string())).ret("ID(f)");

        if(not mgConnection.execute(query)) {
            return std::nullopt;
        }
        auto queryResult = mgConnection->FetchAll();
        if(queryResult && queryResult->front().front().type() == mg::Value::Type::Int) {
            return FileReference(queryResult->front().front().ValueInt());
        }
        return std::nullopt;
    }

    bool insertEdge(NodeReference const & from, NodeReference const & to) const noexcept {
        return mgConnection.executeAndDiscard(
            CipherQuery().match(Node{.name="ff",.label=Label::File}).where("ID(ff)=$fromFile")
            .setInt("fromFile",from.fileRef.fileId)
            .match(Node{.name="ft",.label=Label::File}).where("ID(ft)=$toFile")
            .setInt("toFile",to.fileRef.fileId)
            .merge(Node("f",Label::Settlement,"id:$from"))
            .setInt("from",from.nodeId)
            .merge(Node("t",Label::Settlement,"id:$to"))
            .setInt("to",to.nodeId)
            .merge(Relation{.from=Var("f"),.label=Label::neighbours,.to=Var("t")})
            .merge(Relation{.from=Var("f"),.label=Label::stored,.to=Var("ff")})
            .merge(Relation{.from=Var("t"),.label=Label::stored,.to=Var("ft")})
        );
    }

    bool insertEdge(NodeIdType from, NodeIdType to,FileReference const & fileRef) const noexcept {
        return insertEdge({from,fileRef},{to,fileRef});
    }

    bool insertEdges(fishnet::util::forward_range_of<std::pair<NodeReference,NodeReference>> auto && edges)const noexcept{
        CipherQuery query {"UNWIND $data AS edge "};
        query.match(Node{.name="ff",.label=Label::File}).where("ID(ff)=edge.fromFile");
        query.match(Node{.name="tf",.label=Label::File}).where("ID(tf)=edge.toFile");
        query.merge(Node("f",Label::Settlement,"id:edge.from"));
        query.merge(Node("t",Label::Settlement,"id:edge.to"));
        query.merge(Relation{.from=Var("f"),.label=Label::neighbours,.to=Var("t")});
        query.merge(Relation{.from=Var("f"),.label=Label::stored,.to=Var("ff")});
        query.merge(Relation{.from=Var("t"),.label=Label::stored,.to=Var("tf")});
        std::vector<mg::Value> data;
        for(auto && [from,to]:edges){
            mg::Map currentEdge{4};
            currentEdge.Insert("from",mg::Value(asInt(from.nodeId)));
            currentEdge.Insert("fromFile",mg::Value(asInt(from.fileRef.fileId)));
            currentEdge.Insert("to",mg::Value(asInt(to.nodeId)));
            currentEdge.Insert("toFile",mg::Value(asInt(to.fileRef.fileId)));
            data.push_back(mg::Value(std::move(currentEdge)));
        }
        query.set("data",mg::Value(mg::List(std::move(data))));
        return mgConnection.executeAndDiscard(query);
    }

    bool insertNode(NodeReference const & node) const noexcept{
        return mgConnection.executeAndDiscard(
            CipherQuery().match(Node{.name="f",.label=Label::File}).where("ID(f)=$fid")
            .setInt("fid",node.fileRef.fileId)
            .merge(Node("n",Label::Settlement,"id:$nid"))
            .setInt("nid",node.nodeId)
            .merge(Relation{.from=Var("n"),.label=Label::stored,.to=Var("f")}));
    }

    bool insertNodes(fishnet::util::forward_range_of<NodeReference> auto && nodes) const noexcept {
        CipherQuery query {"UNWIND $data AS node "};
        query.match(Node{.name="f",.label=Label::File}).where("ID(f)=node.fileId");
        query.merge(Node("n",Label::Settlement,"id:node.id"));
        query.merge(Relation{.from=Var("n"),.label=Label::stored,.to=Var("f")});
        std::vector<mg::Value> data;
        for(NodeReference const& node: nodes){
            mg::Map currentNode {2};
            currentNode.Insert("id",mg::Value(asInt(node.nodeId)));
            currentNode.Insert("fileId",mg::Value(asInt(node.fileRef.fileId)));
            data.push_back(mg::Value(std::move(currentNode)));
        }
        query.set("data",mg::Value(mg::List(std::move(data))));
        return mgConnection.executeAndDiscard(query);
    }

    bool removeNode(NodeReference const & node) const noexcept {
        return mgConnection.executeAndDiscard(
            CipherQuery().match(Node("n",Label::Settlement,"id:$id"))
            .setInt("id",node.nodeId)
            .del("n"));
    }

    bool removeNodes(fishnet::util::forward_range_of<NodeReference> auto && nodes) const noexcept {
        CipherQuery query {"UNWIND $data as node "};
        query.match(Node("n",Label::Settlement,"id:node.id")).del("n");
        std::vector<mg::Value> data;
        for(NodeReference const& node: nodes) {
            mg::Map currentNode {1};
            currentNode.Insert("id",mg::Value(asInt(node.nodeId)));
            data.push_back(mg::Value(std::move(currentNode)));
        }
        query.set("data",mg::Value(mg::List(std::move(data))));
        return mgConnection.executeAndDiscard(query);
    }

    bool removeEdge(NodeReference const & from, NodeReference const & to) const noexcept {
        return mgConnection.executeAndDiscard(
            CipherQuery::MATCH(Relation("a",Node("f",Label::Settlement,"id:$fromId"),Label::neighbours,Node("t",Label::Settlement,"id:$toId")))
            .setInt("fromId",from.nodeId)
            .setInt("toId",to.nodeId)
            .del("a"));
    }

    bool removeEdges(fishnet::util::forward_range_of<std::pair<NodeReference,NodeReference>> auto && edges) const noexcept {
        CipherQuery query {"UNWIND $data as edge "};
        query.match(Relation("a",Node("f",Label::Settlement,"id:edge.from"),Label::neighbours,Node("t",Label::Settlement,"id:edge.to")));
        query.del("a");
        std::vector<mg::Value> data;
        for(const auto & [from,to]: edges) {
            mg::Map currentEdge {2};
            currentEdge.Insert("from",mg::Value(asInt(from.nodeId)));
            currentEdge.Insert("to",mg::Value(asInt(to.nodeId)));
            data.push_back(mg::Value(std::move(currentEdge)));
        }
        query.set("data",mg::Value(mg::List(std::move(data))));
        return mgConnection.executeAndDiscard(query);
    }

    std::optional<ComponentReference> createComponent(fishnet::util::forward_range_of<NodeIdType> auto && nodesOfComponent) const noexcept {
        std::vector<mg::Value> data;
        if(fishnet::util::isEmpty(nodesOfComponent))
            return std::nullopt;
        data.reserve(fishnet::util::size(nodesOfComponent));
        std::ranges::transform(nodesOfComponent,std::back_inserter(data),[](NodeIdType nodeId){
            return mg::Value(asInt(nodeId));
        });
        if( mgConnection.execute(CipherQuery("CREATE ")
            .create(Node{.name="c",.label=Label::Component}).endl()
            .append("WITH $data as nodes").endl()
            .append("UNWIND nodes as nodeId").endl()
            .match(Node{.name="n",.label=Label::Settlement}).where("n.id=nodeId")
            .merge(Relation{.from=Var("n"),.label=Label::part_of,.to=Var("c")})
            .ret("ID(c)"))
        ){
            auto queryResult = mgConnection->FetchAll();
            if(queryResult && queryResult->front().front().type() == mg::Value::Type::Int) {
                return ComponentReference(queryResult->front().front().ValueInt());
            }
        }
        return std::nullopt;
    }

    std::vector<ComponentReference> createComponents(const std::vector<std::vector<NodeIdType>> & components) const noexcept {
        CipherQuery query {"WITH $data as components "};
        query.append("UNWIND range(0,size(components)-1) as index").endl();
        query.create(Node{.name="c",.label=Label::Component});
        query.append("WITH components[index] as nodes,c").endl();
        query.append("UNWIND nodes as nodeId").endl();
        query.match(Node{.name="n",.label=Label::Settlement}).where("n.id=nodeId");
        query.merge(Relation{.from=Var("n"),.label=Label::part_of,.to=Var("c")});
        query.ret("DISTINCT ID(c)");
        std::vector<mg::Value> data;
        data.reserve(components.size());
        for(const auto & component: components) {
            std::vector<mg::Value> current;
            current.reserve(component.size());
            std::ranges::transform(component,std::back_inserter(current),[](NodeIdType nodeId){
                return mg::Value(asInt(nodeId));
            });
            data.push_back(mg::Value(mg::List(std::move(current))));
        }
        query.set("data",mg::Value(mg::List(std::move(data))));
        if(mgConnection.execute(query)) {
            std::vector<ComponentReference> result;
            while(auto currentRow = mgConnection->FetchOne()) {
                if(currentRow->front().type() == mg::Value::Type::Int){
                    result.emplace_back(currentRow->front().ValueInt());
                }
            }
            return result;
        }
        return {};
    }

    bool containsNode(size_t nodeId) const noexcept {
        if(mgConnection.execute(CipherQuery().match(Node{"n",Label::Settlement,"id:$id"}).setInt("id",nodeId).ret("ID(n)"))){
                auto result =  mgConnection->FetchAll();
                return result.has_value() && result->size() > 0;
        }
        return false;
    }

    bool containsEdge(size_t from, size_t to) const noexcept {
        if(mgConnection.execute(
            CipherQuery().match(Relation{
                .name="r",
                .from=Node{.label=Label::Settlement,.attributes="id:$fid"},
                .label=Label::neighbours,
                .to= Node{.label=Label::Settlement,.attributes="id:$tid"}
            }).setInt("fid",from).setInt("tid",to).ret("ID(r)"))
        ){
            auto result = mgConnection->FetchAll();
            return result.has_value() && result->size() > 0;
        }
        return false;
    }

    std::vector<NodeIdType> adjacency(const NodeReference & node) const noexcept {
        if(mgConnection.execute(
            CipherQuery().match(Relation{
                .from=Node{.label=Label::Settlement,.attributes="id:$id"},
                .label=Label::neighbours,
                .to=Node{.name="x",.label=Label::Settlement}
            }).setInt("id",node.nodeId).ret("x.id"))
        ){
            std::vector<NodeIdType> output;
            while(auto currentRow = mgConnection->FetchOne()){
                if(currentRow->front().type() == mg::Value::Type::Int){
                    NodeIdType nodeId = asNodeIdType(currentRow->front().ValueInt());
                    output.push_back(nodeId);
                }
            }
            return output;
        }
        return {};
    }

    std::unordered_map<NodeIdType,std::vector<NodeIdType>> edges() const noexcept {
        if(mgConnection.execute(
            CipherQuery().match(Relation{
                .from=Node{.name="f",.label=Label::Settlement},
                .label=Label::neighbours,
                .to=Node{.name="t",.label=Label::Settlement}
            }).ret("f.id","t.id"))
        ){
            std::unordered_map<NodeIdType,std::vector<NodeIdType>> output;
            while(auto currentRow = mgConnection->FetchOne()) {
                NodeIdType from = asNodeIdType(currentRow->at(0).ValueInt());
                NodeIdType to = asNodeIdType(currentRow->at(1).ValueInt());
                if(not output.contains(from))
                    output.try_emplace(from,std::vector<NodeIdType>());
                output.at(from).push_back(to);
            }
            return output;
        }
        return {};
    }

    std::vector<NodeIdType> nodes() const noexcept {
        if(mgConnection.execute(CipherQuery().match(Node{.name="n",.label=Label::Settlement}).ret("n.id"))){
            std::vector<NodeIdType> output;
            while(auto currentRow = mgConnection->FetchOne()) {
                output.push_back(asNodeIdType(currentRow->at(0).ValueInt()));
            }
            return output;
        }
        return {};
    }

    std::vector<NodeIdType> nodesOfComponents(fishnet::util::forward_range_of<ComponentReference> auto && componentIds) const noexcept {
        std::vector<mg::Value> data;
        std::ranges::transform(componentIds,std::back_inserter(data),[](ComponentReference componentRef){
            return mg::Value(componentRef.componentId);
        });
        if(mgConnection.execute(CipherQuery("WITH $data as components").endl()
            .set("data",mg::Value(mg::List(std::move(data))))
            .append("UNWIND components as component_id").endl()
            .match(Node{.name="c",.label=Label::Component}).where("ID(c)=component_id")
            .match(Relation{.from=Node{.name="n",.label=Label::Settlement},.label=Label::part_of,.to=Var("c")})
            .ret("n.id"))
        ){
            std::vector<NodeIdType> result;
            while(auto currentRow = mgConnection->FetchOne()) {
                result.push_back(asNodeIdType(currentRow->at(0).ValueInt()));
            }
            return result;
        }
        return {};
    }

    bool clearAll() const noexcept{
        bool result = true;
        for(Label label: {Label::Settlement,Label::File,Label::Component}){
            result &= mgConnection.executeAndDiscard(CipherQuery().match(Node{.name="n",.label=label}).del("n"));
        }
        return result && dropConstraints() && dropIndexes();
    }
};




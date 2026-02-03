# pragma once
#include <fishnet/Fishnet.hpp>
#include <fishnet/BinaryAdjacency.hpp>
#include <fishnet/FileReference.hpp>

struct HashingFileReferenceMapper {
    FileReference operator()(const fishnet::Shapefile & shapefile) const noexcept {
        return FileReference::hash(shapefile.getPath().stem());
    }
};

template<fishnet::geometry::Shape S>
struct BinarySettlementShapeSerializer {
    static std::vector<uint8_t> operator()(const SettlementShape<S> & settlement){
        std::vector<uint8_t> buffer;
        // Serialize ID
        size_t id = settlement.key();
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&id), reinterpret_cast<const uint8_t*>(&id) + sizeof(size_t));
        // Serialize FileReference
        FileReference fileRef = settlement.file();
        size_t fileId = fileRef.fileId;
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&fileId), reinterpret_cast<const uint8_t*>(&fileId) + sizeof(size_t));
        return buffer;
    }
};

template<fishnet::geometry::Shape S>
struct BinarySettlementShapeDeserializer {
    std::unordered_map<size_t, S> idToGeometryMap;

    BinarySettlementShapeDeserializer(std::vector<SettlementShape<S>> && settlements) {
        for (auto && settlement : settlements) {
            auto key = settlement.key();
            idToGeometryMap.insert({key, std::move(settlement.geometry())});
        }
    }

    BinarySettlementShapeDeserializer() = default;

    SettlementShape<S> operator()(const std::vector<uint8_t> & buffer){
        if (buffer.size() != 2 * sizeof(size_t)) {
            throw std::runtime_error("Invalid buffer size");
        }

        // Deserialize ID
        size_t id;
        std::memcpy(&id, buffer.data(), sizeof(size_t));

        // Deserialize FileReference
        size_t fileId;
        std::memcpy(&fileId, buffer.data() + sizeof(size_t), sizeof(size_t));

        // Create and return the SettlementShape
        auto geometry = std::move(idToGeometryMap.at(id));
        idToGeometryMap.erase(id);
        return SettlementShape<S>(id, FileReference::create(fileId), std::move(geometry));
    }
};

template<fishnet::geometry::Shape S>
class BinarySettlementGraphAdjacency : public fishnet::graph::BinaryAdjacency<fishnet::graph::AdjacencyMap<SettlementShape<S>>> {
private:
    using Base = fishnet::graph::BinaryAdjacency<fishnet::graph::AdjacencyMap<SettlementShape<S>>>;
    std::filesystem::path filePath;
    std::unordered_map<FileReference, std::filesystem::path> fileRefToPathMap;

    void write() const {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing");
        }
        // Serialize fileRefToPathMap
        size_t mapSize = fileRefToPathMap.size();
        file.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
        for (const auto& [fileRef, path] : fileRefToPathMap) {
            size_t fileId = fileRef.fileId;
            file.write(reinterpret_cast<const char*>(&fileId), sizeof(fileId));
            std::string pathStr = fishnet::util::PathHelper::absoluteCanonical(path).string();
            size_t pathSize = pathStr.size();
            file.write(reinterpret_cast<const char*>(&pathSize), sizeof(pathSize));
            file.write(pathStr.c_str(), pathSize);
        }

        // Serialize the graph data
        auto data = this->get();
        file.write(reinterpret_cast<const char*>(data.data()), data.size());

        file.close();
    }

    void read() {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open() || std::filesystem::is_empty(filePath)) {
            return;
        }

        // Deserialize fileRefToPathMap
        size_t mapSize;
        file.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));
        for (size_t i = 0; i < mapSize; ++i) {
            size_t fileId;
            file.read(reinterpret_cast<char*>(&fileId), sizeof(fileId));
            size_t pathSize;
            file.read(reinterpret_cast<char*>(&pathSize), sizeof(pathSize));
            std::string pathStr(pathSize, '\0');
            file.read(&pathStr[0], pathSize);
            fileRefToPathMap[FileReference::create(fileId)] = std::filesystem::path(pathStr);
        }

        // Read the remaining binary data
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // Load the data into the graph
        this->load(data);
    }

public:
    BinarySettlementGraphAdjacency(std::filesystem::path outputPath,std::unordered_map<FileReference, std::filesystem::path> && fileRefToPathMap)
        :Base(
            fishnet::graph::AdjacencyMap<SettlementShape<S>>(),
            BinarySettlementShapeSerializer<S>{},
            BinarySettlementShapeDeserializer<S>{}),
        filePath(std::move(outputPath)),
        fileRefToPathMap(std::move(fileRefToPathMap)){}

    BinarySettlementGraphAdjacency(std::filesystem::path inputPath, std::vector<SettlementShape<S>> && settlements)
        :Base(
            fishnet::graph::AdjacencyMap<SettlementShape<S>>(),
            BinarySettlementShapeSerializer<S>{},
            BinarySettlementShapeDeserializer<S>{std::move(settlements)}),
        filePath(std::move(inputPath))
    {
            read();
    }

    ~BinarySettlementGraphAdjacency() {
        write();
    }
};
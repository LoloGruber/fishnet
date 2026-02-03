#pragma once
#include <memory>
#include <fishnet/CollectionConcepts.hpp>

/**
 * @brief File reference have a unique id for each file
 * 
 */
struct FileReference{
    size_t fileId;

    FileReference(int64_t fileId):fileId(static_cast<size_t>(fileId)){}

    FileReference():fileId(static_cast<size_t>(-1)){}

    operator int64_t() const noexcept {
        return static_cast<int64_t>(fileId);
    }

    static FileReference create(size_t id){
        FileReference fileRef;
        fileRef.fileId = id;
        return fileRef;
    }

    template<fishnet::util::Hashable T>
    static FileReference hash(const T & obj) noexcept {
        FileReference fileRef;
        fileRef.fileId = std::hash<T>{}(obj);
        return fileRef;
    }
};
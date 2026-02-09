#pragma once
#include <memory>
#include <fishnet/CollectionConcepts.hpp>

/**
 * @brief File reference have a unique id for each file
 * 
 */
struct FileReference{
    size_t fileId;

    explicit FileReference(size_t fileId):fileId(fileId){}

    FileReference():fileId(static_cast<size_t>(-1)){}

    static FileReference fromInt(std::integral auto id){
        return FileReference(static_cast<size_t>(id));
    }

    template<fishnet::util::Hashable T>
    static FileReference hash(const T & obj) noexcept {
        FileReference fileRef;
        fileRef.fileId = std::hash<T>{}(obj);
        return fileRef;
    }

    bool operator==(const FileReference & other) const noexcept {
        return this->fileId == other.fileId;
    }
};

namespace std {
    template<>
    struct hash<FileReference> {
        size_t operator()(const FileReference & fileRef) const noexcept {
            return fileRef.fileId;
        }
    };
} // namespace std

static_assert(not std::convertible_to<FileReference, size_t>, "FileReference should not be implicitly convertible to size_t");
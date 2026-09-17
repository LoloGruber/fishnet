#pragma once
#include <fishnet/Concepts.hpp>

/**
 * @brief File reference have a unique id for each file
 * 
 */
struct FileReference{
    size_t fileId;

    explicit FileReference(size_t fileId):fileId(fileId){}

    FileReference():fileId(static_cast<size_t>(-1)){}

    size_t hash() const noexcept {
        return fileId;
    }

    bool operator==(const FileReference & other) const noexcept {
        return this->fileId == other.fileId;
    }

    static FileReference fromInt(std::integral auto id){
        return FileReference(static_cast<size_t>(id));
    }

    template<fishnet::util::Hashable T>
    static FileReference hash(const T & obj) noexcept {
        FileReference fileRef;
        fileRef.fileId = std::hash<T>{}(obj);
        return fileRef;
    }


};

static_assert(not std::convertible_to<FileReference, size_t>, "FileReference should not be implicitly convertible to size_t");
static_assert(fishnet::util::Mapable<FileReference>, "FileReference should be a valid Key type for maps");
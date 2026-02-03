#pragma once
#include <memory>

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
};
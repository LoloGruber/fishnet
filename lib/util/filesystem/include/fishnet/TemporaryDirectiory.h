#pragma once
#include <filesystem>
#include <optional>

namespace fishnet::util{
/**
 * @brief Creates an empty temporary directory at "/tmp/fishnet/{id}/", cleaned by OS or explicitly via TemporaryDirectory::clear()
 * 
 */
class TemporaryDirectory{
private:
    size_t identifier;
    std::filesystem::path directory;
    static std::filesystem::path fromID(size_t id) noexcept;
    static size_t randomUniqueID() noexcept;
    TemporaryDirectory(size_t identifier, std::filesystem::path directory):identifier(identifier),directory(std::move(directory)){}
public:
    const static inline std::filesystem::path TMP_PREFIX = std::filesystem::temp_directory_path() / std::filesystem::path("fishnet/");
    static std::optional<TemporaryDirectory> load(size_t id) noexcept;

    TemporaryDirectory();
    operator const std::filesystem::path & () const noexcept;
    const std::filesystem::path & get() const noexcept;
    size_t id() const noexcept;
    void clear() const noexcept;
    bool init() const noexcept;
};

class AutomaticTemporaryDirectory: public TemporaryDirectory{
public:
    static std::optional<AutomaticTemporaryDirectory> load(size_t id) noexcept;
    ~AutomaticTemporaryDirectory();
};
}
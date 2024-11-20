#include <fishnet/TemporaryDirectiory.h>
#include <stdexcept>
#include <random>

namespace fishnet::util{

std::filesystem::path TemporaryDirectory::fromID(size_t id) noexcept {
    return TMP_PREFIX / std::filesystem::path(std::to_string(id)+"/");
}

size_t TemporaryDirectory::randomUniqueID() noexcept {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::uniform_int_distribution<size_t> distribution(0,std::numeric_limits<size_t>::max());
    size_t id = distribution(generator);
    while(std::filesystem::exists(TemporaryDirectory::fromID(id))) {
        id = distribution(generator);
    }
    return id;
}

std::optional<TemporaryDirectory> TemporaryDirectory::load(size_t id) noexcept{
    auto path = TemporaryDirectory::fromID(id);
    if(std::filesystem::exists(path))
        return std::optional<TemporaryDirectory>(TemporaryDirectory(id,path));
    return std::nullopt;
}

TemporaryDirectory::TemporaryDirectory() :identifier(randomUniqueID()),directory(fromID(identifier)){
    init();
}

TemporaryDirectory::operator const std::filesystem::path & () const noexcept{
    return this->get();
}

const std::filesystem::path & TemporaryDirectory::get()const noexcept{
    return this->directory;
}

size_t TemporaryDirectory::id() const noexcept{
    return this->identifier;
}

void TemporaryDirectory::clear() const noexcept {
    std::filesystem::remove_all(directory);
    std::filesystem::remove(directory);
}

bool TemporaryDirectory::init() const noexcept {
    if(std::filesystem::exists(this->directory))
        return false;
    std::filesystem::create_directories(directory);
    return true;
}

std::optional<AutomaticTemporaryDirectory> AutomaticTemporaryDirectory::load(size_t id) noexcept {
    return TemporaryDirectory::load(id).transform([](auto&& tmp){return static_cast<AutomaticTemporaryDirectory>(tmp);});
}

AutomaticTemporaryDirectory::~AutomaticTemporaryDirectory(){
    this->clear();
}
}
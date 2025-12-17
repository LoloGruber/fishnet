#include <optional>

namespace fishnet::util {

template<typename T>
class Option : public std::optional<T> {
public:
    template<typename... Args>
    Option(Args&&... args) : std::optional<T>(std::forward<Args>(args)...) {}

    T value_or_throw(const std::string& message="No value present!") & {
        if (this->has_value())
            return this->value();
        throw std::runtime_error(message);
    }

    T&& value_or_throw(const std::string& message="No value present!") && {
        if (this->has_value())
            return std::move(this->value());
        throw std::runtime_error(message);
    }
};
}
#pragma once
#include <vector>
#include <optional>
#include <fishnet/HashConcepts.hpp>
#include <algorithm>

namespace fishnet::util {
/**
 * @brief Compile-time map for compile-time configurations
 * 
 * @tparam Key key type
 * @tparam Value value type
 * @tparam Size size of the map
 */
template<typename Key, typename Value,std::size_t Size> requires std::equality_comparable<Key>
struct StaticMap {
    using value_type = std::pair<Key,Value>;
    std::array<value_type,Size> data;

    constexpr auto findKeyPredicate(const Key & key) const noexcept {
        return [&key](const value_type & keyValPair){
            const auto & [k,_] = keyValPair;
            return k == key;
        };
    }

    constexpr bool contains(const Key & key) const noexcept {
        return std::ranges::any_of(data,findKeyPredicate(key));
    }

    constexpr std::optional<Value> get(const Key & key) const noexcept {
        if(not contains(key))
            return std::nullopt;
        return std::ranges::find_if(data,findKeyPredicate(key))->second;
    }

    constexpr bool try_insert(const Key & key, const Value & value) noexcept {
        if(contains(key))
            return false;
        data.emplace_back(key,value);
        return true;
    }

    constexpr bool erase(const Key & key) noexcept {
        if(not contains(key))
            return false;
        data.erase(std::ranges::remove_if(data,findKeyPredicate(key)));
        return true;
    }

    constexpr auto begin() noexcept {
        return data.begin();
    }

    constexpr auto end() noexcept {
        return data.end();
    }

    constexpr auto cbegin() const noexcept {
        return data.cbegin();
    }

    constexpr auto cend() const noexcept {
        return data.cend();
    }
};
}
#pragma once
#include <unordered_map>
#include "HashConcepts.hpp"

namespace util {

template<Mapable OuterKey, Mapable InnerKey, typename Value>
class NestedMap {
private:
    using InnerMap = std::unordered_map<InnerKey,Value>;
    std::unordered_map<OuterKey,InnerMap> map;
    const static inline InnerMap EMPTY_INNER_MAP = InnerMap();

public:
    constexpr std::optional<Value> get(const OuterKey & key, const InnerKey & innerKey) const noexcept {
        if(not map.contains(key) || not map.at(key).contains(innerKey)) {
            return std::nullopt;
        }
        return map.at(key).at(innerKey);
    }

    constexpr auto innerMap(const OuterKey & key ) const noexcept {
        return map.contains(key) ? std::views::all(map.at(key)):std::views::all(EMPTY_INNER_MAP);
    }

    constexpr Value getOrElse(const OuterKey & key, const InnerKey & innerKey, Value defaultValue) const noexcept {
        return get(key,innerKey).value_or(defaultValue);
    }

    constexpr void erase(const OuterKey & key, const InnerKey & innerKey) noexcept {
        if(not map.contains(key))
            return;
        map.at(key).erase(innerKey);
    }

    constexpr void eraseOuterKey(const OuterKey & key)  noexcept {
        map.erase(key);
    }

    constexpr void eraseInnerKey(const InnerKey & key) noexcept {
        std::ranges::for_each(map,[&key](auto  & keyToMapPair){keyToMapPair.second.erase(key);});
    }

    constexpr void try_insert(const OuterKey & key) noexcept {
        if(not map.contains(key)) {
            map.try_emplace(key,InnerMap());
        }
    }

    constexpr void insert(const OuterKey & key, const InnerKey & innerKey, Value value) noexcept {
        if(not map.contains(key)) {
            map.try_emplace(key,InnerMap());
        }
        map.at(key).insert_or_assign(innerKey,value);
    }

    constexpr bool try_insert(const OuterKey & key, const InnerKey & innerKey, Value value) noexcept {
        if(not map.contains(key)) {
            map.try_emplace(key,InnerMap());
        }
        return map.at(key).try_emplace(innerKey,value).second;
    }

    constexpr bool contains(const OuterKey & key, const InnerKey & innerKey) const noexcept {
        return map.contains(key) && map.at(key).contains(innerKey);
    }

    constexpr bool contains(const OuterKey & key) const noexcept {
        return map.contains(key);
    }

    constexpr bool containsOuterKey(const OuterKey & key) const noexcept {
        return map.contains(key);
    }

    constexpr size_t size() const noexcept {
        return map.size();
    }

    constexpr size_t size(const OuterKey & key) const noexcept {
        return map.contains(key) ? map.at(key).size(): 0;
    }
};
}
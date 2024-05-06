#pragma once
#include <unordered_map>
#include <type_traits>
#include "BidirectionalMap.hpp"
namespace fishnet::util {

template<Mapable Key, Mapable AltKey,typename Value>
class AlternativeKeyMap {
private:
    std::unordered_map<Key,Value> map;
    fishnet::util::BidirectionalMap<Key,AltKey> keyMap;

public:
    constexpr std::optional<AltKey> getAlternative(const Key & key) const noexcept {
        return keyMap.getTo(key);
    }

    constexpr std::optional<Key> getKey(const AltKey & key) const noexcept {
        return keyMap.getFrom(key);
    }

    constexpr std::optional<Value> getFromKey(const Key & key) const noexcept {
        if(not map.contains(key))
            return std::nullopt;
        return map.at(key);
    }

    constexpr std::optional<Value> getFromAlternative(const AltKey & secondaryKey) const noexcept {
        auto primary = keyMap.getFrom(secondaryKey);
        if(not primary)
            return std::nullopt;
        return getFromKey(primary.value());
    }

    constexpr Value getFromKeyOrElse(const Key & key, Value defaultValue) const noexcept {
        if(not map.contains(key))
            return defaultValue;
        return map.at(key);
    }

    constexpr Value getFromAlternativeOrElse(const AltKey & secondaryKey, Value defaultValue)const noexcept {
        auto primary = keyMap.getFrom(secondaryKey);
        if(not primary)
            return defaultValue;
        return getFromKeyOrElse(primary.value(),std::move(defaultValue)); // use type index 
    }

    constexpr std::optional<Value> get(const Key & key) const noexcept requires(not std::convertible_to<Key,AltKey>) {
        return getFromKey(key);
    }

    constexpr std::optional<Value> get(const AltKey & key) const noexcept requires(not std::convertible_to<AltKey,Key>){
        return getFromAlternative(key);
    }

    constexpr Value getOrElse(const Key & key, Value defaultValue) const noexcept requires(not std::convertible_to<Key,AltKey>) {
        return getFromKeyOrElse(key,std::move(defaultValue));
    }

    constexpr Value getOrElse(const AltKey & secondaryKey, Value defaultValue) const noexcept requires(not std::convertible_to<Key,AltKey>) {
        return getFromAlternativeOrElse(secondaryKey,std::move(defaultValue));
    }

    constexpr bool containsKey(const Key & key) const noexcept {
        return map.contains(key);
    }

    constexpr bool containsAlternative(const AltKey & key) const noexcept {
        return keyMap.containsTo(key);
    }

    constexpr bool contains(const Key & key) const noexcept requires(not std::convertible_to<Key,AltKey>) {
        return containsPrimary(key);
    }

    constexpr bool contains(const AltKey & key) const noexcept requires(not std::convertible_to<Key,AltKey>) {
        return containsAlternative(key);
    }

    constexpr bool insert(const Key & key, const AltKey & secondaryKey, Value value) noexcept {
        auto valueInsert = map.try_emplace(key,std::move(value));
        if(not valueInsert.second)
            return false;
        keyMap.insert(key,secondaryKey);
        return true;
    }

    constexpr bool updateAlternative(const Key & key, const AltKey & secondaryKey) noexcept {
        if(keyMap.containsTo(secondaryKey) || not map.contains(key))
            return false;
        keyMap.insert(key,secondaryKey);
        return true;
    }

    constexpr bool eraseFromKey(const Key & key) noexcept {
        auto valueErase = map.erase(key);
        if(valueErase == 0)
            return false;
        return keyMap.eraseFrom(key);
    }

    constexpr bool eraseFromAlternative(const AltKey & key) noexcept {
        auto primary = keyMap.getFrom(key);
        if(not primary)
            return false;
        return eraseFromKey(primary.value());
    }

    constexpr bool erase(const Key & key) noexcept requires(not std::convertible_to<Key,AltKey>){
        return erasePrimary(key);
    }

    constexpr bool erase(const AltKey & key) noexcept requires(not std::convertible_to<Key,AltKey>) {
        return eraseFromAlternative(key);
    }

    constexpr auto begin() noexcept {
        return map.begin();
    }

    constexpr auto end() noexcept {
        return map.end();
    }

    constexpr auto begin() const noexcept {
        return map.begin();
    }

    constexpr auto end() const noexcept {
        return map.end();
    }

    constexpr auto cbegin() const noexcept {
        return map.cbegin();
    }

    constexpr auto cend() const noexcept {
        return map.cend();
    }

    constexpr bool empty() const noexcept {
        return map.empty();
    }

    constexpr size_t size() const noexcept {
        return map.size();
    }

    constexpr void clear() noexcept {
        map.clear();
        keyMap.clear();
    }
};

}
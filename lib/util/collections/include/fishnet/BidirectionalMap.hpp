#pragma once
#include <unordered_map>
#include <type_traits>
#include <fishnet/HashConcepts.hpp>
namespace fishnet::util {

/**
 * @brief Bidirectional Map. Behaves like a regular map, but also implements fast reversed look-up in trade for memory
 * 
 * @tparam FromType from type 
 * @tparam ToType to type
 */
template<Mapable FromType, Mapable ToType>
class BidirectionalMap {
private: 
    std::unordered_map<FromType,ToType> map;
    std::unordered_map<ToType,FromType> inverse;
public:
    using value_type = std::pair<const FromType,const ToType>;

    constexpr BidirectionalMap(std::initializer_list<value_type> init){
        for(const auto & [from,to]:init){
            try_insert(from,to);
        }
    }

    constexpr BidirectionalMap()=default;

    constexpr std::optional<ToType> getTo(const FromType & key) const noexcept {
        if(not map.contains(key))
            return std::nullopt;
        return map.at(key);
    }

    constexpr std::optional<FromType> getFrom(const ToType & key) const noexcept {
        if(not inverse.contains(key))
            return std::nullopt;
        return inverse.at(key);
    }

    constexpr std::optional<ToType> get(const FromType & key) const noexcept requires(not std::convertible_to<FromType,ToType>){
        return getTo(key);
    }

    constexpr std::optional<FromType> get(const ToType & key) const noexcept requires(not std::convertible_to<FromType,ToType>) {
        return getFrom(key);
    }

    constexpr bool containsFrom(const FromType & key) const noexcept {
        return map.contains(key);
    }

    constexpr bool containsTo(const ToType & key) const noexcept {
        return inverse.contains(key);
    }

    constexpr bool contains(const FromType & key) const noexcept requires(not std::convertible_to<FromType,ToType>) {
        return containsFrom(key);
    }

    constexpr bool contains(const ToType & key) const noexcept requires(not std::convertible_to<FromType,ToType>) {
        return containsTo(key);
    }

    constexpr bool try_insert(const FromType & fromKey, ToType toKey) noexcept {
        if(map.contains(fromKey) || inverse.contains(toKey))
            return false;
        map.try_emplace(fromKey,toKey);
        inverse.try_emplace(toKey,fromKey);
        return true;
    }

    constexpr void insert(const FromType & fromKey, ToType toKey) noexcept {
        if(map.contains(fromKey)){
            eraseFrom(fromKey);
        }
        if(inverse.contains(toKey)){
            eraseTo(toKey);
        }
        map[fromKey] = toKey;
        inverse[toKey] = fromKey;
    }


    constexpr bool eraseFrom(const FromType & fromKey) noexcept {
        if(not map.contains(fromKey))
            return false;
        inverse.erase(map.at(fromKey));
        map.erase(fromKey);
        return true;
    }

    constexpr bool eraseTo(const ToType & toKey) noexcept {
        if(not inverse.contains(toKey))
            return false;
        map.erase(inverse.at(toKey));
        inverse.erase(toKey);
        return true;
    }

    constexpr bool erase(const FromType & fromKey) noexcept requires(not std::convertible_to<FromType,ToType>) {
        return eraseFrom(fromKey);
    }

    constexpr bool erase(const ToType & toKey) noexcept requires(not std::convertible_to<FromType,ToType>) {
        return eraseTo(toKey);
    }

    constexpr auto begin() noexcept {
        return map.begin();
    }

    constexpr auto end() noexcept {
        return map.end();
    }

    constexpr auto cbegin() const noexcept {
        return map.cbegin();
    }

    constexpr auto cend() const noexcept {
        return map.cend();
    }

    constexpr auto inverseBegin() noexcept {
        return inverse.begin();
    }

    constexpr auto inverseEnd() noexcept {
        return inverse.end();
    }

    constexpr auto cInverseBegin() const noexcept {
        return inverse.cbegin();
    }

    constexpr auto cInverseEnd() const noexcept {
        return inverse.cend();
    }

    constexpr bool empty() const noexcept {
        return map.empty();
    }

    constexpr size_t size() const noexcept {
        return map.size();
    }

    constexpr void clear() noexcept {
        map.clear();
        inverse.clear();
    }
};
}
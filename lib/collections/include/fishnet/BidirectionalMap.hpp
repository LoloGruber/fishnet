#pragma once
#include <unordered_map>
#include <type_traits>
#include <fishnet/CollectionConcepts.hpp>
namespace fishnet::util {

namespace __impl{

template<typename Expected, typename Actual>
concept UniversalType = std::convertible_to<std::remove_cvref_t<Expected>, std::remove_cvref_t<Actual>>;

template<typename Derived, typename MapType, typename InverseMapType>
class AbstractBidirectionalMap {
public:
    using value_type = std::pair<const typename MapType::key_type, const typename MapType::mapped_type>;
    using value_type_non_const = std::pair<typename MapType::key_type, typename MapType::mapped_type>;
    using inverse_value_type = std::pair<const typename InverseMapType::key_type, const typename InverseMapType::mapped_type>;
    using from_type = typename MapType::key_type;
    using to_type = typename MapType::mapped_type;
    static_assert(std::is_same_v<from_type, typename InverseMapType::mapped_type> && std::is_same_v<to_type, typename InverseMapType::key_type>, "Map and inverse map types are not consistent");
protected:
    MapType map;
    InverseMapType inverse;
    
    constexpr void update(value_type && value) noexcept {
        static_cast<Derived*>(this)->updateImpl(std::forward<decltype(value)>(value));
    }
public:

    constexpr AbstractBidirectionalMap()=default;

    AbstractBidirectionalMap(std::initializer_list<value_type_non_const> init){
        for(auto && value :init){
            insert(std::move(value));
        }
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

    constexpr bool containsFrom(const from_type & key) const noexcept {
        return map.contains(key);
    }

    constexpr bool containsTo(const to_type & key) const noexcept {
        return inverse.contains(key);
    }

    template<typename T>
    constexpr bool contains(const T & key) const noexcept 
        requires(UniversalType<T, from_type> || UniversalType<T, to_type>) 
            && (not std::convertible_to<from_type,to_type>) 
    {
        if constexpr(UniversalType<T, from_type>) {
            return containsFrom(key);
        } else {
            return containsTo(key);
        }
    }

    constexpr void insert(UniversalType<value_type> auto && value) noexcept {
        update(std::forward<decltype(value)>(value));
    }

    constexpr void insert(UniversalType<from_type> auto && fromKey, UniversalType<to_type> auto && toKey) noexcept {
        insert(value_type{std::forward<decltype(fromKey)>(fromKey), std::forward<decltype(toKey)>(toKey)});
    }

    constexpr bool try_insert(UniversalType<value_type> auto && value) noexcept {
        if(contains(value.first) || containsTo(value.second))
            return false;
        insert(std::forward<decltype(value)>(value));
        return true;
    }

    constexpr bool try_insert(UniversalType<from_type> auto && fromKey, UniversalType<to_type> auto && toKey) noexcept {
        return try_insert(value_type{std::forward<decltype(fromKey)>(fromKey), std::forward<decltype(toKey)>(toKey)});
    }

    constexpr bool eraseFrom(const from_type & fromKey) noexcept {
        if(not map.contains(fromKey))
            return false;
        auto [begin,end] = map.equal_range(fromKey);
        for(auto it = begin; it != end; ++it) {
            inverse.erase(it->second);
        }
        map.erase(fromKey);
        return true;
    }

    constexpr bool eraseTo(const to_type & toKey) noexcept {
        if(not inverse.contains(toKey))
            return false;
        auto [begin,end] = inverse.equal_range(toKey);
        for(auto it = begin; it != end; ++it) {
            map.erase(it->second);
        }
        inverse.erase(toKey);
        return true;
    }

    template<typename T>
    constexpr bool erase(const T & key) noexcept 
        requires(UniversalType<T, from_type> || UniversalType<T, to_type>) 
            && (not std::convertible_to<from_type,to_type>) 
    {
        if constexpr(UniversalType<T, from_type>) {
            return eraseFrom(key);
        } else {
            return eraseTo(key);
        }
    }

    constexpr void clear() noexcept {
        map.clear();
        inverse.clear();
    }

    ~AbstractBidirectionalMap() = default;
};

}

/**
 * @brief Bidirectional Hash Map. Behaves like a regular map, but also implements fast reversed look-up in trade for memory
 * 
 * @tparam from_type from type 
 * @tparam to_type to type
 */
template<Mapable from_type, Mapable to_type>
class BidirectionalHashMap: public __impl::AbstractBidirectionalMap<BidirectionalHashMap<from_type,to_type>, std::unordered_map<from_type,to_type>, std::unordered_map<to_type,from_type>> {
private:
    using Base = __impl::AbstractBidirectionalMap<BidirectionalHashMap<from_type,to_type>, std::unordered_map<from_type,to_type>, std::unordered_map<to_type,from_type>>;
    friend Base;

    constexpr void updateImpl(typename Base::value_type && value) noexcept {
        if(this->map.contains(value.first)){
            this->eraseFrom(value.first);
        }
        if(this->inverse.contains(value.second)){
            this->eraseTo(value.second);
        }
        this->inverse.insert(inverse_value_type{value.second,value.first});
        this->map.insert(std::forward<decltype(value)>(value));
    }
public:
    using value_type = typename Base::value_type;
    using inverse_value_type = typename Base::inverse_value_type;

    BidirectionalHashMap(std::initializer_list<typename Base::value_type_non_const> init):Base(std::move(init)) {}

    constexpr BidirectionalHashMap()=default;

    constexpr std::optional<to_type> getTo(const from_type & key) const noexcept {
        if(not this->map.contains(key))
            return std::nullopt;
        return this->map.at(key);
    }

    constexpr std::optional<from_type> getFrom(const to_type & key) const noexcept {
        if(not this->inverse.contains(key))
            return std::nullopt;
        return this->inverse.at(key);
    }

    template<typename T>
    constexpr auto get(const T & key) const noexcept 
        requires(__impl::UniversalType<T, from_type> || __impl::UniversalType<T, to_type>) 
            && (not std::convertible_to<from_type,to_type>) 
    {
        if constexpr(__impl::UniversalType<T, from_type>) {
            return getTo(key);
        } else {
            return getFrom(key);
        }
    }
};


/**
 * @brief Bidirectional Hash MultiMap. Behaves like a regular multimap, but also implements fast reversed look-up in trade for memory
 * 
 * @tparam from_type from type 
 * @tparam to_type to type
 */
template<Mapable from_type, Mapable to_type>
class BidirectionalHashMultiMap: public __impl::AbstractBidirectionalMap<BidirectionalHashMultiMap<from_type,to_type>, std::unordered_multimap<from_type,to_type>, std::unordered_multimap<to_type,from_type>> {
private:
    using Base = __impl::AbstractBidirectionalMap<BidirectionalHashMultiMap<from_type,to_type>, std::unordered_multimap<from_type,to_type>, std::unordered_multimap<to_type,from_type>>;

    constexpr void updateImpl(typename Base::value_type && value) noexcept {
        this->inverse.insert(inverse_value_type{value.second,value.first});
        this->map.insert(std::forward<decltype(value)>(value));
    }

public:
    using value_type = typename Base::value_type;
    using inverse_value_type = typename Base::inverse_value_type;

    BidirectionalHashMultiMap(std::initializer_list<typename Base::value_type_non_const> init):Base(std::move(init)) {}

    constexpr BidirectionalHashMultiMap()=default;

    constexpr auto getTo(const from_type & key) const noexcept {
        if(not this->map.contains(key))
            return std::nullopt;
        return std::make_optional(std::ranges::subrange(this->map.equal_range(key)));
    }

    constexpr auto getFrom(const to_type & key) const noexcept {
        if(not this->inverse.contains(key))
            return std::nullopt;
        return std::make_optional(std::ranges::subrange(this->inverse.equal_range(key)));
    }

    constexpr auto get(const from_type & key) const noexcept requires(not std::convertible_to<from_type,to_type>){
        return getTo(key);
    }

    constexpr auto get(const to_type & key) const noexcept requires(not std::convertible_to<from_type,to_type>) {
        return getFrom(key);
    }
};
}
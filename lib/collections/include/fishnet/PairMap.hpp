#pragma once
#include "HashConcepts.hpp"
#include "CantorPairing.hpp"

namespace fishnet::util {
template<Mapable Key, Mapable SecondKey>
class DualKey {
    private:
        Key key;
        SecondKey secondKey;
    public:
        constexpr DualKey(const Key & key, const SecondKey & secondKey):key(key),secondKey(secondKey){}

        constexpr DualKey(Key && key, SecondKey && secondKey):key(std::forward<Key>(key)),secondKey(std::forward<SecondKey>(secondKey)){}

        constexpr std::pair<Key,SecondKey> keys() const noexcept {
            return std::make_pair(key,secondKey);
        }

        constexpr const Key & getKey() const noexcept {
            return key;
        }

        constexpr const SecondKey & getSecondKey() const noexcept {
            return secondKey;
        }

        constexpr bool operator==(const DualKey<Key,SecondKey> & other ) const noexcept {
            return key == other.getKey() && secondKey == other.getSecondKey();
        }
};
}
namespace std {
    template<typename K,typename S>
    struct hash<fishnet::util::DualKey<K,S>>{
        constexpr static hash<K> keyHasher {};
        constexpr static hash<S> secondKeyHasher {};
        size_t operator()(const fishnet::util::DualKey<K,S> & dualKey) const noexcept {
            size_t keyHash = keyHasher(dualKey.getKey());
            size_t secondKeyHash = secondKeyHasher(dualKey.getSecondKey());
            return fishnet::util::CantorPairing(keyHash,secondKeyHash);
        }
    };
}   

namespace fishnet::util {

template<Mapable K1, Mapable K2, typename Value>
using pair_map = std::unordered_map<DualKey<K1,K2>,Value>;
}
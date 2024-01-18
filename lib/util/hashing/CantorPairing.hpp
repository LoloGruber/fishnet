#pragma once

namespace util{

constexpr inline size_t CantorPairing(size_t x, size_t y) noexcept {
    return ((x+y+1)*(x+y))/2 +y;
}
}
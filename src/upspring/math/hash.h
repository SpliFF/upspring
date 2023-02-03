#ifndef UPSPRING_MATH_HASH_H
#define UPSPRING_MATH_HASH_H

#include <functional>

// Happen's to be jochumdev's birthday.
static const std::size_t HASH_SEED = 0x603;

inline void hash_combine(std::size_t& seed) { }

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    hash_combine(seed, rest...);
}

#endif // UPSPRING_MATH_HASH_H
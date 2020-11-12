#ifndef BITS_HPP_INCLUDED
#define BITS_HPP_INCLUDED

#include <cstdint>

namespace Utils {

inline constexpr unsigned long long to_kib(unsigned long long bytes) { return bytes*1024; }
inline constexpr unsigned long long to_mib(unsigned long long bytes) { return bytes*1024*1024; }
inline constexpr unsigned long long to_gib(unsigned long long bytes) { return bytes*1024*1024*1024; }
inline constexpr unsigned long long to_tib(unsigned long long bytes) { return bytes*1024*1024*1024*1024; }

inline constexpr unsigned long long to_kb(unsigned long long bytes) { return bytes*1000; }
inline constexpr unsigned long long to_mb(unsigned long long bytes) { return bytes*1000*1000; }
inline constexpr unsigned long long to_gb(unsigned long long bytes) { return bytes*1000*1000*1000; }
inline constexpr unsigned long long to_tb(unsigned long long bytes) { return bytes*1000*1000*1000*1000; }

inline constexpr unsigned long long to_kibit(unsigned long long bytes) { return bytes*1024/8; }
inline constexpr unsigned long long to_mibit(unsigned long long bytes) { return bytes*1024*1024/8; }
inline constexpr unsigned long long to_gibit(unsigned long long bytes) { return bytes*1024*1024*1024/8; }
inline constexpr unsigned long long to_tibit(unsigned long long bytes) { return bytes*1024*1024*1024*1024/8; }

inline uint32_t rotl32(const uint32_t x, const int k)
{
    return (x << k) | (x >> (32 - k));
}

inline uint32_t rotr32(const uint32_t x, const int k)
{
    return (x >> k) | (x << (32 - k));
}

} // namespace Utils

#endif

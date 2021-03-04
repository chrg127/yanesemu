#ifndef BITS_HPP_INCLUDED
#define BITS_HPP_INCLUDED

#include <cstdint>

namespace Util {

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

constexpr inline uint32_t rotl32(const uint32_t x, const int k)
{
    return (x << k) | (x >> (32 - k));
}

constexpr inline uint32_t rotr32(const uint32_t x, const int k)
{
    return (x >> k) | (x << (32 - k));
}

constexpr inline uint64_t set_bits(uint64_t num, uint8_t shift, uint64_t mask, uint64_t data)
{
    // if mask == 1, this could've been optimized with !!data instead of (data & mask)
    // (in practive, i'm not sure if it's really worth it...)
    return (num & ~(mask << shift)) | (data & mask) << shift;
}

constexpr inline uint64_t set_bit(uint64_t num, uint8_t bit, bool data)
{
    return (num & ~(1UL << bit)) | data << bit;
}

constexpr inline uint64_t get_mask_nbits(unsigned nbits)
{
    return (1UL << nbits) - 1UL;
}

/* A struct for portable bit-fields. Use it like so:
 * union {
 *     uint16_t full
 *     BitField<1, uint16_t> flag;
 *     BitField<2, uint16_t> flag2;
 *     // ...
 * } data;
 * the types must necessarily be the same or else it won't work at all.
 */
template <typename T, unsigned Bitno, unsigned Nbits = 1>
struct BitField {
    T data;
    static constexpr uint64_t MASK = get_mask_nbits(Nbits);

    // this operator= below is ESSENTIAL. before adding it i got a bunch of bugs
    // when copying BitFields with the same exact types (i don't know why it
    // wouldn't call the second operator= here.
    BitField & operator=(const BitField<T, Bitno, Nbits> &oth) { data = set_bits(data, Bitno, MASK, oth.data); return *this; }
    BitField & operator=(const T val)                          { data = set_bits(data, Bitno, MASK, val);      return *this; }
    operator uint64_t() const                                  { return (data >> Bitno) & MASK; }

    template <typename U> BitField & operator+=(const U val) { *this = *this + val; return *this; }
    template <typename U> BitField & operator-=(const U val) { *this = *this - val; return *this; }
    template <typename U> BitField & operator*=(const U val) { *this = *this * val; return *this; }
    template <typename U> BitField & operator/=(const U val) { *this = *this / val; return *this; }
    template <typename U> BitField & operator&=(const U val) { *this = *this & val; return *this; }
    template <typename U> BitField & operator|=(const U val) { *this = *this | val; return *this; }
    template <typename U> BitField & operator^=(const U val) { *this = *this ^ val; return *this; }
    template <typename U> BitField & operator>>=(const U val) { *this = *this >> val; return *this; }
    template <typename U> BitField & operator<<=(const U val) { *this = *this << val; return *this; }

    BitField & operator++()    { return *this = *this + 1; }
    // BitField & operator++(int) { auto r = *this; ++*this; return r; }
    BitField & operator--()    { return *this = *this - 1; }
    // BitField & operator--(int) { auto r = *this; ++*this; return r; }
};

} // namespace Util

#endif

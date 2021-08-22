#pragma once

/* Bit manipulation library. */

#include <cstdint>

namespace util {

/* Returns a mask usable to mask off a given number of bits.
 * For example: 3 -> 0b11; 6 -> 0b111111 */
constexpr inline uint64_t bitmask(uint8_t nbits)
{
    return (1UL << nbits) - 1UL;
}

/* Routines for getting/settings bits at once. More readable, and thus
 * preferable, than using naked bit operations. */
constexpr inline uint64_t getbits(uint64_t num, uint8_t bitno, uint8_t nbits)
{
    return num >> bitno & bitmask(nbits);
}

constexpr inline uint64_t getbit(uint64_t num, uint8_t bitno)
{
    return getbits(num, bitno, 1);
}

constexpr inline uint64_t setbits(uint64_t num, uint8_t bitno, uint8_t nbits, uint64_t data)
{
    const uint64_t mask = bitmask(nbits);
    return (num & ~(mask << bitno)) | (data & mask) << bitno;
}

constexpr inline uint64_t setbit(uint64_t num, uint8_t bitno, bool data)
{
    return setbits(num, bitno, 1, data);
}

constexpr inline uint8_t reverse_bits(uint8_t n)
{
    n = (n & 0xF0) >> 4 |  (n & 0x0F) << 4;
    n = (n & 0xCC) >> 2 |  (n & 0x33) << 2;
    n = (n & 0xAA) >> 1 |  (n & 0x55) << 1;
    return n;
}

/* A struct for portable bit-fields. Use it like so:
 * union {
 *     uint16_t full
 *     BitField<uint16_t, 1, 1> flag;
 *     BitField<uint16_t, 2, 3> flag_3bits;
 *     // ...
 * } data;
 * the types must necessarily be the same or else it won't work at all. */
template <typename T, unsigned Bitno, unsigned Nbits = 1>
struct BitField {
    T data;

    BitField() = default;
    BitField(const BitField<T, Bitno, Nbits> &b)               { operator=(b); }
    // this operator= below is ESSENTIAL. before adding it I got a bunch
    // of bugs when copying BitFields with the same exact types.
    BitField & operator=(const BitField<T, Bitno, Nbits> &oth) { data = setbits(data, Bitno, Nbits, uint64_t(oth)); return *this; }
    BitField & operator=(const uint64_t val)                   { data = setbits(data, Bitno, Nbits, val);           return *this; }
    operator uint64_t() const                                  { return (data >> Bitno) & bitmask(Nbits); }

    template <typename U> BitField & operator+= (const U val) { *this = *this +  val; return *this; }
    template <typename U> BitField & operator-= (const U val) { *this = *this -  val; return *this; }
    template <typename U> BitField & operator*= (const U val) { *this = *this *  val; return *this; }
    template <typename U> BitField & operator/= (const U val) { *this = *this /  val; return *this; }
    template <typename U> BitField & operator&= (const U val) { *this = *this &  val; return *this; }
    template <typename U> BitField & operator|= (const U val) { *this = *this |  val; return *this; }
    template <typename U> BitField & operator^= (const U val) { *this = *this ^  val; return *this; }
    template <typename U> BitField & operator>>=(const U val) { *this = *this >> val; return *this; }
    template <typename U> BitField & operator<<=(const U val) { *this = *this << val; return *this; }

    BitField & operator++() { return *this = *this + 1; }
    BitField & operator--() { return *this = *this - 1; }
};

// A 16 number that can be accessed either through its full value and through its low and high byte.
union Word {
    uint16_t v;
    struct { uint8_t l; uint8_t h; };

    Word() = default;
    Word(uint16_t val) : v(val) {}
    Word & operator= (uint16_t val) { v  = val; return *this; }
    Word & operator&=(uint64_t val) { v &= val; return *this; }
    Word & operator|=(uint64_t val) { v |= val; return *this; }
};

// Unit converters.
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

} // namespace Util

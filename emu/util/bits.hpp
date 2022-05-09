#pragma once

#include <concepts>
#include "common.hpp"

namespace bits {

// Returns a mask usable to mask off a given number of bits.
// For example: 3 -> 0b11; 6 -> 0b111111
constexpr inline u64 bitmask(u8 nbits)
{
    return (1UL << nbits) - 1UL;
}

constexpr inline u64 getbits(u64 num, u8 bitno, u8 nbits)
{
    return num >> bitno & bitmask(nbits);
}

constexpr inline u64 getbit(u64 num, u8 bitno)
{
    return num >> bitno & 1;
}

constexpr inline u64 setbits(u64 num, u8 bitno, u8 nbits, u64 data)
{
    const u64 mask = bitmask(nbits);
    return (num & ~(mask << bitno)) | (data & mask) << bitno;
}

constexpr inline u64 setbit(u64 num, u8 bitno, u8 data)
{
    return (num & ~(1 << bitno)) | (data & 1) << bitno;
}

constexpr inline u8 reverse(u8 n)
{
    n = (n & 0xF0) >> 4 |  (n & 0x0F) << 4;
    n = (n & 0xCC) >> 2 |  (n & 0x33) << 2;
    n = (n & 0xAA) >> 1 |  (n & 0x55) << 1;
    return n;
}

/*
 * A struct for portable bit-fields. Use it like so:
 * union {
 *     u16 full
 *     BitField<u16, 1, 1> flag;
 *     BitField<u16, 2, 3> flag_3bits;
 *     // ...
 * } data;
 * the types must necessarily be the same or else it won't work at all.
 */
template <std::integral T, unsigned Bitno, unsigned Nbits = 1>
struct BitField {
    T data;

    BitField() = default;
    BitField(const BitField<T, Bitno, Nbits> &b)               { operator=(b); }
    BitField & operator=(const BitField<T, Bitno, Nbits> &oth) { data = setbits(data, Bitno, Nbits, u64(oth)); return *this; }
    BitField & operator=(const u64 val)                   { data = setbits(data, Bitno, Nbits, val);           return *this; }
    operator u64() const                                  { return (data >> Bitno) & bitmask(Nbits); }

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

    constexpr unsigned bitno() const { return Bitno; }
    constexpr unsigned nbits() const { return Nbits; }
};

// A 16 number that can be accessed either through its full value and through its low and high byte.
union Word {
    u16 v;
    struct { u8 l, h; };

    Word() = default;
    explicit Word(u16 val) : v(val) {}
    Word & operator= (u16 val) { v  = val; return *this; }
    Word & operator&=(u64 val) { v &= val; return *this; }
    Word & operator|=(u64 val) { v |= val; return *this; }
};

namespace literals {
    inline constexpr std::size_t operator"" _KiB(unsigned long long bytes) { return bytes*1024; }
    inline constexpr std::size_t operator"" _MiB(unsigned long long bytes) { return bytes*1024*1024; }
    inline constexpr std::size_t operator"" _GiB(unsigned long long bytes) { return bytes*1024*1024*1024; }
    inline constexpr std::size_t operator"" _TiB(unsigned long long bytes) { return bytes*1024*1024*1024*1024; }

    inline constexpr std::size_t operator"" _KB(unsigned long long bytes) { return bytes*1000; }
    inline constexpr std::size_t operator"" _MB(unsigned long long bytes) { return bytes*1000*1000; }
    inline constexpr std::size_t operator"" _GB(unsigned long long bytes) { return bytes*1000*1000*1000; }
    inline constexpr std::size_t operator"" _TB(unsigned long long bytes) { return bytes*1000*1000*1000*1000; }

    inline constexpr std::size_t operator"" _Kib(unsigned long long bytes) { return bytes*1024/8; }
    inline constexpr std::size_t operator"" _Mib(unsigned long long bytes) { return bytes*1024*1024/8; }
    inline constexpr std::size_t operator"" _Gib(unsigned long long bytes) { return bytes*1024*1024*1024/8; }
    inline constexpr std::size_t operator"" _Tib(unsigned long long bytes) { return bytes*1024*1024*1024*1024/8; }
} // namespace literals

} // namespace bits

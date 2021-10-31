#pragma once

/*
 * This header file provides aliases for precise bit size types (uint8_t, uint16_t,
 * etc.) and a class for arbitrary bit sizes (uint1, uint2, ..., uint63).
 */

#include <cstdint>
#include <type_traits>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

template <unsigned Bits>
struct UInt {
    static_assert(Bits <= 64);
    using IntType = std::conditional_t<Bits <= 8 , u8,
                    std::conditional_t<Bits <= 16, u16,
                    std::conditional_t<Bits <= 32, u32,
                    std::conditional_t<Bits <= 64, u64,
                    void>>>>;
private:
    IntType num = 0;

    inline IntType cast(const IntType val) { return val & (~0ull >> (64 - Bits)); }

public:
    UInt() = default;

    template <std::integral T>
    UInt(const T val) { operator=(val); }

    static constexpr IntType MAX = (~0ull >> (64 - Bits));

    // Permits conversions and casts from UInt<Bits> to IntType
    operator IntType() const { return num; }
    IntType value() const    { return num; }

    UInt operator=  (const IntType val) { num =   cast(val); return *this; }
    UInt operator+= (const IntType val) { num +=  cast(val); return *this; }
    UInt operator-= (const IntType val) { num -=  cast(val); return *this; }
    UInt operator*= (const IntType val) { num *=  cast(val); return *this; }
    UInt operator/= (const IntType val) { num /=  cast(val); return *this; }
    UInt operator&= (const IntType val) { num &=  cast(val); return *this; }
    UInt operator|= (const IntType val) { num |=  cast(val); return *this; }
    UInt operator%= (const IntType val) { num %=  cast(val); return *this; }
    UInt operator^= (const IntType val) { num ^=  cast(val); return *this; }
    UInt operator>>=(const IntType val) { num >>= cast(val); return *this; }
    UInt operator<<=(const IntType val) { num <<= cast(val); return *this; }

    UInt operator++(int) { auto ret = *this; num = cast(num + 1); return ret;   }
    UInt operator++()    {                   num = cast(num + 1); return *this; }
    UInt operator--(int) { auto ret = *this; num = cast(num - 1); return ret;   }
    UInt operator--()    {                   num = cast(num - 1); return *this; }
};

template <unsigned Bits, typename T>
inline UInt<Bits> operator&(UInt<Bits> n, T val)
{
    n &= val;
    return n;
}

// Convenient aliases
using u1 = UInt<1>; using u2 = UInt<2>; using u3 = UInt<3>;
using u4 = UInt<4>; using u5 = UInt<5>; using u6 = UInt<6>;
using u7 = UInt<7>;

using u9  = UInt<9>;  using u10 = UInt<10>; using u11 = UInt<11>;
using u12 = UInt<12>; using u13 = UInt<13>; using u14 = UInt<14>;
using u15 = UInt<15>;

using u17 = UInt<17>; using u18 = UInt<18>; using u19 = UInt<19>;
using u20 = UInt<20>; using u21 = UInt<21>; using u22 = UInt<22>;
using u23 = UInt<23>; using u24 = UInt<24>; using u25 = UInt<25>;
using u26 = UInt<26>; using u27 = UInt<27>; using u28 = UInt<28>;
using u29 = UInt<29>; using u30 = UInt<30>; using u31 = UInt<31>;

using u33 = UInt<33>; using u34 = UInt<34>; using u35 = UInt<35>;
using u36 = UInt<36>; using u37 = UInt<37>; using u38 = UInt<38>;
using u39 = UInt<39>; using u40 = UInt<40>; using u41 = UInt<41>;
using u42 = UInt<42>; using u43 = UInt<43>; using u44 = UInt<44>;
using u45 = UInt<45>; using u46 = UInt<46>; using u47 = UInt<47>;
using u48 = UInt<48>; using u49 = UInt<49>; using u50 = UInt<50>;
using u51 = UInt<51>; using u52 = UInt<52>; using u53 = UInt<53>;
using u54 = UInt<54>; using u55 = UInt<55>; using u56 = UInt<56>;
using u57 = UInt<57>; using u58 = UInt<58>; using u59 = UInt<59>;
using u60 = UInt<60>; using u61 = UInt<61>; using u62 = UInt<62>;
using u63 = UInt<63>;

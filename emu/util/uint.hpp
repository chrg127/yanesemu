#pragma once

/*
 * This header file provides aliases for precise bit size types (uint8_t, uint16_t,
 * etc.) and a class for arbitrary bit sizes (uint1, uint2, ..., uint63).
 */

#include <cstdint>
#include <type_traits>

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

template <unsigned Bits>
struct UInt {
    static_assert(Bits <= 64);
    using inttype = std::conditional_t<Bits <= 8 , uint8,
                    std::conditional_t<Bits <= 16, uint16,
                    std::conditional_t<Bits <= 32, uint32,
                    std::conditional_t<Bits <= 64, uint64,
                    void>>>>;
private:
    inttype num = 0;

    inline inttype cast(const inttype val) { return val & (~0ull >> (64 - Bits)); }

public:
    UInt() = default;
    template <typename T> UInt(const T val) { operator=(val); }

    static constexpr inttype MAX = (~0ull >> (64 - Bits));

    // Permits conversions and casts from UInt<Bits> to inttype
    operator inttype() const { return num; }
    inttype value() const    { return num; }

    template <typename T> UInt operator=  (const T val) { num =   cast(val); return *this; }
    template <typename T> UInt operator+= (const T val) { num +=  cast(val); return *this; }
    template <typename T> UInt operator-= (const T val) { num -=  cast(val); return *this; }
    template <typename T> UInt operator*= (const T val) { num *=  cast(val); return *this; }
    template <typename T> UInt operator/= (const T val) { num /=  cast(val); return *this; }
    template <typename T> UInt operator&= (const T val) { num &=  cast(val); return *this; }
    template <typename T> UInt operator|= (const T val) { num |=  cast(val); return *this; }
    template <typename T> UInt operator%= (const T val) { num %=  cast(val); return *this; }
    template <typename T> UInt operator^= (const T val) { num ^=  cast(val); return *this; }
    template <typename T> UInt operator>>=(const T val) { num >>= cast(val); return *this; }
    template <typename T> UInt operator<<=(const T val) { num <<= cast(val); return *this; }

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

// Helpers.
using uint1 = UInt<1>; using uint2 = UInt<2>; using uint3 = UInt<3>;
using uint4 = UInt<4>; using uint5 = UInt<5>; using uint6 = UInt<6>;
using uint7 = UInt<7>;

using uint9 = UInt<9>;   using uint10 = UInt<10>; using uint11 = UInt<11>;
using uint12 = UInt<12>; using uint13 = UInt<13>; using uint14 = UInt<14>;
using uint15 = UInt<15>;

using uint17 = UInt<17>; using uint18 = UInt<18>; using uint19 = UInt<19>;
using uint20 = UInt<20>; using uint21 = UInt<21>; using uint22 = UInt<22>;
using uint23 = UInt<23>; using uint24 = UInt<24>; using uint25 = UInt<25>;
using uint26 = UInt<26>; using uint27 = UInt<27>; using uint28 = UInt<28>;
using uint29 = UInt<29>; using uint30 = UInt<30>; using uint31 = UInt<31>;

using uint33 = UInt<33>; using uint34 = UInt<34>; using uint35 = UInt<35>;
using uint36 = UInt<36>; using uint37 = UInt<37>; using uint38 = UInt<38>;
using uint39 = UInt<39>; using uint40 = UInt<40>; using uint41 = UInt<41>;
using uint42 = UInt<42>; using uint43 = UInt<43>; using uint44 = UInt<44>;
using uint45 = UInt<45>; using uint46 = UInt<46>; using uint47 = UInt<47>;
using uint48 = UInt<48>; using uint49 = UInt<49>; using uint50 = UInt<50>;
using uint51 = UInt<51>; using uint52 = UInt<52>; using uint53 = UInt<53>;
using uint54 = UInt<54>; using uint55 = UInt<55>; using uint56 = UInt<56>;
using uint57 = UInt<57>; using uint58 = UInt<58>; using uint59 = UInt<59>;
using uint60 = UInt<60>; using uint61 = UInt<61>; using uint62 = UInt<62>;
using uint63 = UInt<63>;
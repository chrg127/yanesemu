#ifndef CORE_TYPES_HPP_INCLUDED
#define CORE_TYPES_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <emu/util/unsigned.hpp>

namespace Core {

union Reg16 {
    struct {
        uint8 low, high;
    };
    uint16 reg;

    Reg16() : reg(0)
    { }
    Reg16(uint16 val)
    { operator=(val); }

    inline void operator=(const uint16 val)
    {
        reg = val;
    }
    
    template <typename T> // T = numeric type
    inline Reg16 & operator&=(const T val)
    { reg &= val; return *this; }

    template <typename T> // T = numeric type
    inline Reg16 & operator|=(const T val)
    { reg |= val; return *this; }
};

template <typename T> // T = numeric type
inline Reg16 operator&(Reg16 left, T right)
{ return left & right; }

template <typename T> // T = numeric type
inline Reg16 operator|(Reg16 left, T right)
{ return left & right; }

/* a ROM represents readable memory. It has two states: at the start, it is
 * both readable and writable. After a function is done with writing, call
 * lock() to lock the ROM and make it read-only. A ROM can be reset(), but
 * doing so will erase the contents. */
class ROM {
    uint8 *mem = nullptr;
    uint32 size = 0;
    bool locked = false;

public:
    ROM() = default;
    ROM(const uint32 s)
    { alloc(s); }
    ~ROM()
    { reset(); }

    ROM(const ROM &) = delete;
    ROM(ROM &&rom)
    { operator=(std::move(rom)); }
    ROM & operator=(const ROM &) = delete;
    ROM & operator=(ROM &&rom)
    {
        mem = rom.mem;
        size = rom.size;
        locked = rom.locked;
        rom.mem = nullptr;
        rom.size = 0;
        rom.locked = false;
        return *this;
    }

    explicit operator bool() const
    { return locked; }

    inline uint8 *getmem() const
    { return locked ? nullptr : mem; }
    inline uint32 getsize() const
    { return size; }

    inline void alloc(const uint32 s)
    {
        if (!locked) {
            mem = new uint8[s];
            size = s;
        }
    }

    inline void reset()
    {
        if (mem)
            delete[] mem;
        mem = nullptr;
        size = 0;
        locked = false;
    }

    inline void write(const uint32 i, const uint8 data)
    {
        if (!locked)
            mem[i] = data;
    }

    inline void lock()
    {
        locked = true;
    }

    inline uint8 operator[](const uint8 addr) const
    { return mem[addr]; }

    inline void copy_to(uint8 *buf, const uint32 start = 0, const uint32 len = 0) const
    {
        std::memcpy(buf, mem+start, len);
    }
};

struct uint24 {
    uint8 high, mid, low;

    uint24() : high(0), mid(0), low(0)
    { }
    uint24(const uint32 val)
    { operator=(val); }

    inline uint32 value()
    { return high << 16 | mid << 8 | low; }

    inline void operator=(const uint32 val)
    {
        high = val & 0x00FF0000;
        mid  = val & 0x0000FF00;
        low  = val & 0x000000FF;
    }
};

} // namespace Core

#endif

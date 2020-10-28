#ifndef CORE_TYPES_HPP_INCLUDED
#define CORE_TYPES_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

namespace Core {

union Reg16 {
    struct {
        uint8_t low, high;
    };
    uint16_t reg;

    Reg16() : reg(0) { }
    Reg16(uint16_t val) : reg(val) { }
    inline void operator=(uint16_t val)
    {
        reg = val;
    }
};

/* a ROM represents readable memory. It has two states: at the start, it is
 * both readable and writable. After a function is done with writing, call
 * lock() to lock the ROM and make it read-only. A ROM can be reset(), but
 * doing so will erase the contents. */
class ROM {
    uint8_t *mem = nullptr;
    uint32_t size = 0;
    bool locked = false;

public:
    ROM() = default;
    ROM(const uint32_t s)
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

    inline uint8_t *getmem() const
    { return locked ? nullptr : mem; }
    inline uint32_t getsize() const
    { return size; }

    inline void alloc(const uint32_t s)
    {
        if (!locked) {
            mem = new uint8_t[s];
            size = s;
        }
    }

    inline void reset()
    {
        if (mem)
            delete mem;
        mem = nullptr;
        size = 0;
        locked = false;
    }

    inline void write(const uint32_t i, const uint8_t data)
    {
        if (!locked)
            mem[i] = data;
    }

    inline void lock()
    {
        locked = true;
    }

    inline uint8_t operator[](const uint8_t addr) const
    { return mem[addr]; }

    inline void copy_to(uint8_t *buf, const uint32_t start = 0, const uint32_t len = 0) const
    {
        std::memcpy(buf, mem+start, len);
    }
};

} // namespace Core

#endif

/* a ROM represents readable memory. It has two states: at the start, it is
 * both readable and writable. After a function is done with writing, call
 * lock() to lock the ROM and make it read-only. A ROM can be reset(), but
 * doing so will erase the contents. */

#include <cstddef>
#include <cstdint>
#include <utility>

class ROM {
    uint8_t *mem = nullptr;
    std::size_t size = 0;
    bool locked = false;

public:
    ROM() = default;
    ROM(std::size_t s)
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

    explicit operator bool()
    { return locked; }

    inline std::size_t getsize() const
    { return size; }

    inline void alloc(std::size_t s)
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

    inline void write(std::size_t i, uint8_t data)
    {
        if (!locked)
            mem[i] = data;
    }

    inline void lock()
    { locked = true; }

    inline uint8_t operator[](uint8_t addr)
    { return mem[addr]; }
};


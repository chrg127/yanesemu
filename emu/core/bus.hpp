#ifndef BUS_H_INCLUDED
#define BUS_H_INCLUDED

#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <emu/core/memorymap.hpp>

namespace Core {

class Bus {
    uint8_t *memory;

public:
    bool write_enable = false;

    Bus() : write_enable(false)
    {
        memory = new uint8_t[MEMSIZE];
    }

    ~Bus()
    {
        delete[] memory;
    }

    void initmem(uint8_t *prgrom, size_t romsize);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    void memdump(FILE *dumpfile);
    void reset()
    { }
};

} // namespace Core

#endif

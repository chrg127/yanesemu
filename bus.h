#ifndef BUS_H_INCLUDED
#define BUS_H_INCLUDED

#include <cstdint>
#include <cstddef>
#include "memorymap.h"

namespace Processor {

class Bus {
    uint8_t memory[MEMSIZE];

public:
    bool write_enable = false;

    Bus() : write_enable(false) { }
    ~Bus() { }

    void initmem(uint8_t *prgrom, size_t romsize);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    void memdump(const char * const fname);
    void reset()
    { }
};

} // namespace Processor

#endif

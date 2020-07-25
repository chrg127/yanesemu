#ifndef BUS_H_INCLUDED
#define BUS_H_INCLUDED

#include <cstdint>
#include "memorymap.h"

class Bus {
    uint8_t memory[MEMSIZE];

public:
    Bus() { }
    ~Bus() { }
    
    void initmem(uint8_t *prgrom);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    void memdump(const char * const fname);
};

#endif

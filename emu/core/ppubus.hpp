#ifndef PPUBUS_HPP_INCLUDED
#define PPUBUS_HPP_INCLUDED

#include <functional>
#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>

namespace Core {

class PPUBus {
    uint8_t memory[PPUMap::MEMSIZE];
    std::function <uint8_t (uint16_t)> get_nt_addr;

public:
    PPUBus(int mirroring);

    void initmem(uint8_t *chrrom);
    uint8_t & operator[](const uint16_t addr);

    // interface to dump()
    uint8_t *getmemory() const
    { return memory; }
    size_t getmemsize() const
    { return PPUMap::MEMSIZE; }
};

} // namespace Core

#endif

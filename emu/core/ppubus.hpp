#ifndef PPUBUS_HPP_INCLUDED
#define PPUBUS_HPP_INCLUDED

#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>

// forward decls
namespace IO { class File; }

namespace Core {

class PPUBus {
    uint8_t *memory;

public:
    PPUBus() : memory(new uint8_t[PPUMap::MEMSIZE])
    { }

    ~PPUBus()
    {
        delete[] memory;
    }

    PPUBus(const PPUBus &) = delete;
    PPUBus(const PPUBus &&) = delete;
    void operator=(const PPUBus &) = delete;

    void initmem(uint8_t *chrrom);
    void memdump(IO::File &df);
    uint8_t &operator[](const uint16_t addr);
};

} // namespace Core

#endif

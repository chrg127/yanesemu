#ifndef PPU_HPP_INCLUDED
#define PPU_HPP_INCLUDED

#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>
#include <emu/io/file.hpp>
#include "ppubus.hpp"
#include "oam.hpp"
#include "background.hpp"

namespace Core {

// forward decls
class PPUBus;

class PPU {
    PPUBus bus;
    OAM oam;
    Background bg;

    // ppu's internal data bus. gets filled on reading and writing to regs.
    uint8_t io_latch;
    struct {
        bool toggle = 0; // used by PPUSCROLL and PPUADDR
    } latch;
    bool nmi_enabled;
    bool ext_bus_dir;
    struct {
        bool greyscale, red, green, blue;
    } effects;
    bool vblank;
    bool spr0hit;
    bool sprov;

    int cycle;

    void scanline_render();
    void scanline_empty();
    inline int getrow()
    {
        return cycle/340;
    }
    inline int getcol()
    {
        return cycle%340;
    }

public:
    PPU()
    { }

    PPU(const PPU &) = delete;
    PPU(PPU &&) = delete;
    PPU &operator=(const PPU &) = delete;
    PPU &operator=(PPU &&) = delete;

    ~PPU()
    { }

    void power(uint8_t *chrrom);
    void reset();
    void main();
    uint8_t readreg(const uint16_t which);
    void writereg(const uint16_t which, const uint8_t data);
    void printinfo(IO::File &log);
};

} // namespace Core

#endif

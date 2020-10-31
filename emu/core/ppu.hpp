#ifndef CORE_PPU_HPP_INCLUDED
#define CORE_PPU_HPP_INCLUDED

#include <functional>
#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>
#include <emu/utils/file.hpp>

namespace Core {

class PPU {

#include <emu/core/vram.hpp>
#include <emu/core/background.hpp>
#include <emu/core/oam.hpp>

    VRAM vram;
    OAM oam;
    Background bg;

    // ppu's internal data bus. gets filled on reading and writing to regs.
    uint8_t io_latch;
    struct {
        bool toggle; // used by PPUSCROLL and PPUADDR
    } latch;
    bool nmi_enabled;
    bool ext_bus_dir;
    struct {
        bool grey, red, green, blue;
    } effects;
    bool vblank;
    bool spr0hit;
    bool sprov;
    bool odd_frame;

    int cycle;

    void fetch_nt();
    void fetch_at();
    void fetch_lowbg();
    void fetch_highbg();
    void bg_cycle();
    void dot256();

    // void scanline_render();
    // void scanline_empty();
    inline int get_x()
    { return cycle/340; }
    inline int get_y()
    { return cycle%340; }

    friend class Background;
    friend class OAM;
    friend class PPUBus;

public:
    PPU(int mirroring) : vram(mirroring)
    {
        bg.vram = &vram;
    }

    void power(const ROM &chrrom);
    void reset();
    void main();
    uint8_t readreg(const uint16_t which);
    void writereg(const uint16_t which, const uint8_t data);
    void printinfo(Utils::File &log);

    inline const uint8_t *getmemory() const
    { return vram.memory; }
    inline uint32_t getmemsize() const
    { return PPUMap::MEMSIZE; }
};

} // namespace Core

#endif

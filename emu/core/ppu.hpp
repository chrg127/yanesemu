#ifndef CORE_PPU_HPP_INCLUDED
#define CORE_PPU_HPP_INCLUDED

#include <functional>
#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>
#include <emu/utils/file.hpp>

namespace Core {

class PPU {
    unsigned long cycles = 0;

#include "vram.hpp"
#include "background.hpp"
#include "oam.hpp"

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

    void begin_frame();
    void cycle_fetchnt(bool cycle);
    void cycle_fetchattr(bool cycle);
    void cycle_fetchlowbg(bool cycle);
    void cycle_fetchhighbg(bool cycle);
    void cycle_incvhorz();
    void cycle_incvvert();
    void cycle_copyhoriz();
    void vblank_begin();
    void vblank_end();
    void copy_vert();

    void fetch_nt();
    void fetch_at();
    void fetch_lowbg();
    void fetch_highbg();

    friend class Background;
    friend class OAM;
    friend class PPUBus;

public:
    PPU()
    { }

    void power(const ROM &chrrom, int mirroring);
    void reset();
    void main();
    uint8_t readreg(const uint16_t which);
    void writereg(const uint16_t which, const uint8_t data);

    // for ppumain.cpp
    template <unsigned int Cycle>
    void ccycle();
    template <unsigned int Line>
    void lcycle(unsigned int cycle);
    template <unsigned Cycle>
    void background_cycle();
    void idlec();

    inline const uint8_t *getmemory() const
    { return vram.memory; }
    inline uint32_t getmemsize() const
    { return PPUMap::MEMSIZE; }
    inline unsigned long get_cycles() const
    { return cycles; }
};

} // namespace Core

#endif

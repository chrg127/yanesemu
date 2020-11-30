#ifndef CORE_PPU_HPP_INCLUDED
#define CORE_PPU_HPP_INCLUDED

#include <functional>
#include <emu/core/types.hpp>
#include <emu/core/memorymap.hpp>
#include <emu/core/genericbus.hpp>

namespace Core {

class PPU {
    friend class Background;
    friend class OAM;
    friend class PPUBus;

    unsigned long cycles = 0;
    unsigned long lines  = 0;
    uint8   screen[256*224];
    uint8   io_latch;
    bool    nmi_enabled;
    bool    ext_bus_dir;
    bool    vblank;
    bool    spr0hit;
    bool    spr_ov;
    bool    odd_frame;
    struct {
        bool grey, red, green, blue;
    } effects;

    struct VRAM {
        PPU     &ppu;
        Bus     bus = Bus(0x4000);
        uint16  v, t;   // vram address, temporary vram address
        uint8   fine_x, inc, readbuf;
        bool    toggle; // used by PPUSCROLL and PPUADDR

        VRAM(PPU &p) : ppu(p) { }
        void power(const ROM &chrrom, int mirroring);
        void reset();
        void inc_horzpos();
        void inc_vertpos();
        void copy_horzpos();
        void copy_vertpos();
        uint8 read(uint16 a)
        { return bus.read(a); }
        uint8 read();
        uint8 readdata();
        void write(uint16 a, uint8 d)
        { bus.write(d, a); }
        void write(uint8 data);
        void writedata(uint8 data);
    } vram;

    struct Background {
        PPU &ppu;

        bool    patterntab_addr;
        uint8   nt_base_addr;
        bool    show;
        bool    show_leftmost;
        struct {
            uint16 bglow, bghigh;
            uint8  atlow, athigh;
            bool   latchlow, latchhigh;
        } shift;
        struct {
            uint8 nt, attr, lowbg, hibg;
        } latch;

        Background(PPU &p) : ppu(p) { }
        void power();
        void reset();
        void shift_fill();
        void shift_run();
        void fetch_nt(bool dofetch);
        void fetch_attr(bool dofetch);
        void fetch_lowbg(bool dofetch);
        void fetch_highbg(bool dofetch);
        uint8 output();
    } bg;

    struct OAM {
        PPU     &ppu;
        uint8   oam[PPUMap::OAM_SIZE];
        bool    sprsize;
        bool    patterntab_addr;
        bool    show;
        bool    show_leftmost;
        uint8   addr;
        uint8   data;
        uint8   shifts[8];
        uint8   latches[8];
        uint8   counters[8];

        OAM(PPU &p) : ppu(p) { }
        void power();
        void reset();
        // uint8 read(uint16 addr);
        // void write(uint16 addr, uint8 data);
    } oam;

    void begin_frame();
    void cycle_fetchnt(bool cycle);
    void cycle_fetchattr(bool cycle);
    void cycle_fetchlowbg(bool cycle);
    void cycle_fetchhighbg(bool cycle);
    void cycle_incvhorz();
    void cycle_incvvert();
    void cycle_copyhorz();
    void cycle_copyvert();
    void cycle_shift();
    void cycle_fillshifts();
    void cycle_outputpixel();
    void vblank_begin();
    void vblank_end();
    void output();
    uint8 getcolor(bool select, uint8 pal, uint8 palind);

public:
    PPU() : vram(*this), bg(*this), oam(*this)
    { }

    void power(const ROM &chrrom, int mirroring);
    void reset();
    void main();
    uint8 readreg(const uint16 which);
    void writereg(const uint16 which, const uint8 data);

    // for ppumain.cpp
    template <unsigned int Cycle>
    void ccycle();
    template <unsigned int Line>
    void lcycle(unsigned int cycle);
    template <unsigned Cycle>
    void background_cycle();
    void idlec();

    inline const uint8 *getmemory() const
    { return vram.bus.memory(); }
    inline uint32 getmemsize() const
    { return 0x4000; }
};

} // namespace Core

#endif

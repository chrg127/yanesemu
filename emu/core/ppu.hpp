#ifndef CORE_PPU_HPP_INCLUDED
#define CORE_PPU_HPP_INCLUDED

#include <array>
#include <functional>
#include <string>
#include <emu/core/types.hpp>
#include <emu/core/memmap.hpp>
#include <emu/core/bus.hpp>

namespace Video { class Canvas; }

namespace Core {

class CPU;
class Cartridge;

class PPU {
    Bus *bus;
    Bus *cpubus;

    Video::Canvas *screen;
    // uint8 screen[SCREEN_WIDTH*SCREEN_HEIGHT];
    unsigned long cycles = 0;
    unsigned long lines  = 0;
    int mirroring = 0;

    uint8   io_latch;
    // whether the ppu can go into vblank (leftmost bit of PPUCTRL)
    bool    nmi_enabled;
    // whether vblank has started, or the leftmost bit of PPUSTATUS
    bool    vblank;
    bool    ext_bus_dir;
    bool    spr0hit;
    bool    spr_ov;
    bool    odd_frame;
    struct {
        bool grey, red, green, blue;
    } effects;

    struct VRAM {
        PPU &ppu;
        std::array<uint8, VRAM_SIZE> mem;
        uint16 v, t; // vram address, temporary vram address
        uint8 fine_x, inc, readbuf;
        bool toggle; // used by PPUSCROLL and PPUADDR

        VRAM(PPU &p) : ppu(p) { }
        void inc_horzpos();
        void inc_vertpos();
        void copy_horzpos();
        void copy_vertpos();
    } vram;

    struct Background {
        PPU &ppu;
        bool    pt_addr;
        uint8   nt_addr; // 2 bits wide
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
        void shift_fill();
        void shift_run();
        void fetch_nt(bool dofetch);
        void fetch_attr(bool dofetch);
        void fetch_lowbg(bool dofetch);
        void fetch_highbg(bool dofetch);
        uint8 output();
    } bg;

    struct OAM {
        std::array<uint8, OAM_SIZE> oammem;
        bool sprsize;
        bool pt_addr;
        bool show;
        bool show_leftmost;
        uint8 addr;
        uint8 shifts[8], latches[8], counters[8];
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
    PPU() : vram(*this), bg(*this)
    { }

    void power();
    void reset();
    void run();
    uint8 readreg(const uint16 which);
    void writereg(const uint16 which, const uint8 data);
    std::string get_info();
    void attach_bus(Bus *pb, Bus *cb);

    // for ppumain.cpp
    template <unsigned int Cycle> void ccycle();
    template <unsigned int Line> void lcycle(unsigned int cycle);
    template <unsigned Cycle> void background_cycle();
    void idlec();

    const std::array<uint8, VRAM_SIZE> & getmemory() const { return vram.mem; }
    void set_mirroring(int m) { mirroring = m; }
    void set_screen(Video::Canvas *canvas) { screen = canvas; }
};

} // namespace Core

#endif

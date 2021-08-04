#pragma once

#include <functional>
#include <string>
#include <emu/core/screen.hpp>
#include <emu/core/bus.hpp>
#include <emu/core/const.hpp>
#include <emu/util/array.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/util/bits.hpp>

class Screen;
template <std::size_t Size> class Bus;
namespace Debugger {
    class Debugger;
    class PPUDebugger;
}

namespace Core {

class PPU {
public:
    union VRAMAddress {
        uint16 v = 0;
        Util::BitField<uint16, 12, 3> fine_y;
        Util::BitField<uint16, 10, 2> nt;
        Util::BitField<uint16, 5,  5> coarse_y;
        Util::BitField<uint16, 0,  5> coarse_x;

        VRAMAddress() = default;
        VRAMAddress(uint16 data) : v(data) { }
        VRAMAddress(const VRAMAddress &a)             { operator=(a); }
        VRAMAddress & operator=(const VRAMAddress &a) { v = a.v; return *this; }

        explicit operator uint16() const            { return v; }
        VRAMAddress & operator+=(uint16 n) { v += n; v = Util::getbits(v, 0, 15); return *this; }

        uint14 as_u14() { return v; }
    };

private:
    Bus<PPUBUS_SIZE> *bus;
    Screen *screen;
    unsigned long cycles = 0;
    unsigned long lines  = 0;
    std::function<void(bool)> nmi_callback;
    bool odd_frame;

    struct IO {
        // used as internal buffer with regs I/O
        uint8 latch = 0;

        // PPUCTRL
        // nt addr is the most significant bits of vram.addr/vram.tmp
        bool vram_inc;
        bool sp_pt_addr;
        bool bg_pt_addr;
        bool sp_size;
        // i can't find explanations for this one
        bool ext_bus_dir;
        // whether the ppu can go into vblank (leftmost bit of PPUCTRL)
        bool nmi_enabled;

        // PPUMASK
        bool grey;
        bool bg_show_left;
        bool sp_show_left;
        bool bg_show;
        bool sp_show;
        bool red;
        bool green;
        bool blue;

        // PPUSTATUS
        bool sp_overflow;
        bool sp_zero_hit;
        bool vblank;

        // PPUSCROLL, PPUADDR
        bool scroll_latch;

        // PPUDATA
        uint8 data_buf;
    } io;

    struct VRAM {
        VRAMAddress addr;
        VRAMAddress tmp;
        uint3 fine_x;
        uint8 buf; // buffer used during background and sprite fetches while rendering
        // simply indicates the VRAM address position. not used anywhere in the PPU though.
        uint16 vx() const { return fine_x      | addr.coarse_x << 3 | (addr.nt & 1) << 8; }
        uint8 vy() const  { return addr.fine_y | addr.coarse_y << 3 | (addr.nt & 2) << 8; }
    } vram;

    struct Tile {
        uint8 nt;
        uint8 attr;
        uint8 low;
        uint8 high;
    } tile;

    struct Shift {
        uint8  attr_low, attr_high;
        bool   feed_low, feed_high;
        uint16 tile_low, tile_high;
    } shift;

    struct OAM {
        uint8 addr;
        uint8 data;

        uint2 sp_counter; // if sp_counter == 0, then mem[addr] points to a sprite's y byte
        bool inrange;

        bool read_ff;
        bool addr_overflow;

        bool sp0_next;
        bool sp0_curr;

        uint8 pattern_low[8], pattern_high[8], attrs[8], xpos[8];
        std::array<uint8, OAM_SIZE> mem;

        void inc()  { ++addr; ++sp_counter; }
        uint8 read() { return read_ff ? 0xFF : mem[addr]; }
    } oam;

    struct {
        uint8 index;
        std::array<uint8, 8*4> mem;

        bool full()           { return index == 32; }
        void write(uint8 val) { if (!full()) mem[index++] = val; }
    } secondary_oam;

    struct Sprite {
        uint8 y;
        uint8 tile;
        uint8 attr;
        uint8 x;
    } sprite;

public:
    PPU(Bus<PPUBUS_SIZE> *vrambus, Screen *scr)
        : bus(vrambus), screen(scr)
    { }

    void power(bool reset);
    void bus_map(Bus<CPUBUS_SIZE> &bus);
    void on_nmi(auto &&callback) { nmi_callback = callback; }

    // ppumain.cpp
    void run();

private:
    uint8 readreg(uint16 addr);
    void writereg(uint16 addr, uint8 data);

    void copy_v_horzpos();
    void copy_v_vertpos();

    uint8 fetch_nt(uint15 vram_addr);
    uint8 fetch_attr(uint16 nt, uint16 coarse_y, uint16 coarse_x);
    uint8 fetch_pt(bool base, uint8 nt, bool bitplane, uint3 fine_y);

    void background_shift_run();
    void background_shift_fill();
    std::pair<uint2, uint2> background_output();

    void sprite_shift_run();
    void sprite_update_flags(unsigned line);
    std::tuple<uint2, uint2, bool> sprite_output(unsigned x);

    uint8 output(unsigned x);
    void render();

    // ppumain.cpp
    template <unsigned Cycle> void background_fetch_cycle();
    template <unsigned Cycle> void sprite_fetch_cycle(unsigned line);
    template <unsigned Cycle> void sprite_fetch_cycle2();
    // template <unsigned Cycle> void sprite_eval(unsigned line);
    template <unsigned Cycle> void ccycle(unsigned line);
    template <unsigned Line> void lcycle(unsigned cycle, void (PPU::*)(unsigned));
    void begin_frame();
    void vblank_begin();
    void vblank_end();

    friend class Debugger::Debugger;
    friend class Debugger::PPUDebugger;
};

} // namespace Core

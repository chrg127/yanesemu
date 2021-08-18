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

        explicit operator uint16() const   { return v; }
        VRAMAddress & operator+=(uint16 n) { v += n; v = Util::getbits(v, 0, 15); return *this; }

        uint14 as_u14() { return v; }
    };

private:
    Bus<PPUBUS_SIZE> *bus;
    Screen *screen;
    unsigned cycles = 0;
    unsigned lines  = 0;
    std::function<void(bool)> nmi_callback;
    bool odd_frame;

    struct {
        // used as internal buffer with regs I/O
        uint8 latch = 0;

        // PPUCTRL
        bool vram_inc;
        bool sp_pt_addr;
        bool bg_pt_addr;
        uint8 sp_size;
        bool ext_bus_dir; // this one is weird
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

    struct {
        VRAMAddress addr;
        VRAMAddress tmp;
        uint3 fine_x;
        uint8 buf; // buffer used during background and sprite fetches while rendering
        // simply indicates the VRAM address position. not used anywhere in the PPU though.
        uint16 vx() const { return fine_x      | addr.coarse_x << 3 | (addr.nt & 1) << 8; }
        uint8 vy() const  { return addr.fine_y | addr.coarse_y << 3 | (addr.nt & 2) << 8; }
    } vram;

    struct {
        uint8 nt;
        uint8 attr;
        uint8 pt_low;
        uint8 pt_high;
    } tile;

    struct {
        uint8  attr_low, attr_high;
        bool   feed_low, feed_high;
        uint16 pt_low, pt_high;
    } shift;

    struct {
        uint8 addr;
        uint8 data;
        uint2 sp_counter = 0; // if sp_counter == 0, then mem[addr] points to a sprite's y byte
        bool inrange = 0;
        bool read_ff = 0;
        bool addr_overflow = 0;
        bool sp0_next = 0;
        bool sp0_curr = 0;
        uint8 pt_low[8], pt_high[8], attrs[8], xpos[8];
        std::array<uint8, OAM_SIZE> mem;

        void inc()  { ++addr; ++sp_counter; }
        uint8 read() { return read_ff ? 0xFF : mem[addr]; }
    } oam;

    struct {
        uint8 index = 0;
        std::array<uint8, 8*4> mem;

        bool full()           { return index == 32; }
        void write(uint8 val) { if (!full()) mem[index] = val; }
        void inc()            { if (!full()) index++; }
    } secondary_oam;

    struct {
        uint8 y;
        uint8 nt;
        uint8 attr;
        uint8 x;
    } sprite;

public:
    PPU(Bus<PPUBUS_SIZE> *vrambus, Screen *scr)
        : bus(vrambus), screen(scr)
    { }

    void power(bool reset);
    uint8 readreg(uint16 addr);
    void writereg(uint16 addr, uint8 data);
    void on_nmi(auto &&callback) { nmi_callback = callback; }

    // ppumain.cpp
    void run();

private:
    void cycle_inc()
    {
        cycles = (cycles + 1) % PPU_MAX_LCYCLE;
        lines += (cycles == 0);
        lines %= PPU_MAX_LINES;
    }

    void copy_v_horzpos();
    void copy_v_vertpos();

    uint8 fetch_nt(uint15 vram_addr);
    uint8 fetch_attr(uint16 nt, uint16 coarse_y, uint16 coarse_x);
    uint8 fetch_pt(bool base, uint8 nt, bool bitplane, uint3 fine_y);
    uint8 fetch_pt_sprite(bool sp_size, uint8 nt, bool bitplane, unsigned row);

    void background_shift_run();
    void background_shift_fill();
    std::pair<uint2, uint2> background_output();

    void sprite_shift_run();
    void sprite_update_flags(unsigned line);
    std::tuple<uint2, uint2, uint8> sprite_output(unsigned x);

    uint8 output(unsigned x);
    void render();

    // ppumain.cpp
    template <unsigned Cycle> void background_fetch_cycle();
    template <unsigned Cycle> void sprite_fetch_cycle(uint3 n, unsigned line);
    template <unsigned Cycle> void sprite_read_secondary();
    template <unsigned Cycle> void cycle(unsigned line);
    template <unsigned Line>  void line(unsigned cycle, void (PPU::*)(unsigned));
    void vblank_begin();
    void vblank_end();

    friend class Debugger::Debugger;
    friend class Debugger::PPUDebugger;
};

} // namespace Core

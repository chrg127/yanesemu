#pragma once

#include <functional>
#include <string>
#include <emu/core/screen.hpp>
#include <emu/core/bus.hpp>
#include <emu/core/const.hpp>
#include <emu/util/array.hpp>
#include <emu/util/uint.hpp>
#include <emu/util/bits.hpp>

class Screen;
template <std::size_t Size> class Bus;
namespace debugger { class PPUDebugger; }

namespace core {

class PPU {
public:
    union VRAMAddress {
        u16 v = 0;
        bits::BitField<u16, 12, 3> fine_y;
        bits::BitField<u16, 10, 2> nt;
        bits::BitField<u16, 5,  5> coarse_y;
        bits::BitField<u16, 0,  5> coarse_x;

        VRAMAddress() = default;
        VRAMAddress(u16 data) : v(data) { }
        VRAMAddress(const VRAMAddress &a)             { operator=(a); }
        VRAMAddress & operator=(const VRAMAddress &a) { v = a.v; return *this; }

        explicit operator u16() const   { return v; }
        VRAMAddress & operator+=(u16 n) { v += n; v = bits::getbits(v, 0, 15); return *this; }

        u14 as_u14() { return v; }
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
        u8 latch = 0;

        // PPUCTRL
        bool vram_inc;
        bool sp_pt_addr;
        bool bg_pt_addr;
        u8 sp_size;
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

        // PPUScroll, PPUAddr
        bool scroll_latch;

        // PPUData
        u8 data_buf;
    } io;

    struct {
        VRAMAddress addr;
        VRAMAddress tmp;
        u3 fine_x;
        u8 buf; // buffer used during background and sprite fetches while rendering
    } vram;

    struct {
        u8 nt;
        u8 attr;
        u8 pt_low;
        u8 pt_high;
    } tile;

    struct {
        u8  attr_low, attr_high;
        bool   feed_low, feed_high;
        u16 pt_low, pt_high;
    } shift;

    struct {
        u8 addr;
        u8 data;
        u2 sp_counter = 0; // if sp_counter == 0, then mem[addr] points to a sprite's y byte
        bool inrange = 0;
        bool read_ff = 0;
        bool addr_overflow = 0;
        bool sp0_next = 0;
        bool sp0_curr = 0;
        u8 pt_low[8], pt_high[8], attrs[8], xpos[8];
        std::array<u8, OAM_SIZE> mem;

        void inc()  { ++addr; ++sp_counter; }
        u8 read() { return read_ff ? 0xFF : mem[addr]; }
    } oam;

    struct {
        u8 index = 0;
        std::array<u8, 8 * 4> mem;

        bool full()           { return index == 32; }
        void write(u8 val) { if (!full()) mem[index] = val; }
        void inc()            { if (!full()) index++; }
    } secondary_oam;

    struct {
        u8 y;
        u8 nt;
        u8 attr;
        u8 x;
    } sprite;

public:
    PPU(Bus<PPUBUS_SIZE> *vrambus, Screen *scr)
        : bus(vrambus), screen(scr)
    { }

    void power(bool reset);
    u8 readreg(u16 addr);
    void writereg(u16 addr, u8 data);
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

    u8 fetch_nt(u15 vram_addr);
    u8 fetch_attr(u16 nt, u16 coarse_y, u16 coarse_x);
    u8 fetch_pt(bool base, u8 nt, bool bitplane, u3 fine_y);
    u8 fetch_pt_sprite(bool sp_size, u8 nt, bool bitplane, unsigned row);

    void background_shift_run();
    void background_shift_fill();
    std::pair<u2, u2> background_output();

    void sprite_shift_run();
    void sprite_update_flags(unsigned line);
    std::tuple<u2, u2, u8> sprite_output(unsigned x);

    u8 output(unsigned x);
    void render();

    // ppumain.cpp
    template <unsigned Cycle> void background_fetch_cycle();
    template <unsigned Cycle> void sprite_fetch_cycle(u3 n, unsigned line);
    template <unsigned Cycle> void sprite_read_secondary();
    template <unsigned Cycle> void cycle(unsigned line);
    template <unsigned Line>  void line(unsigned cycle, void (PPU::*)(unsigned));
    void vblank_begin();
    void vblank_end();

    friend class debugger::PPUDebugger;
};

} // namespace core

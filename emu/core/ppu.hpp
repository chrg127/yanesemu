#ifndef CORE_PPU_HPP_INCLUDED
#define CORE_PPU_HPP_INCLUDED

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
    Bus<PPUBUS_SIZE> *bus;
    Screen *screen;
    unsigned long cycles = 0;
    unsigned long lines  = 0;
    std::function<void(void)> nmi_callback;
    bool odd_frame;

    union VRAMAddress {
        uint16 value = 0;
        Util::BitField<uint16, 12, 3> fine_y;
        Util::BitField<uint16, 10, 2> nt;
        Util::BitField<uint16, 5,  5> coarse_y;
        Util::BitField<uint16, 0,  5> coarse_x;

        VRAMAddress() = default;
        VRAMAddress(uint16 data) : value(data) { }
        VRAMAddress & operator=(const VRAMAddress &addr) { value = addr.value; return *this; }
        operator uint16() const                          { return value; }
        VRAMAddress & operator+=(uint16 n)               { value += n; return *this; }
    };

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

        // OAMADDR
        uint8 oam_addr;

        // PPUSCROLL, PPUADDR
        bool scroll_latch;

        // PPUDATA
        uint8 data_buf;
    } io;

    struct VRAM {
        VRAMAddress addr;
        VRAMAddress tmp;
        uint8 fine_x;
        // 10 bit numbers?
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
        uint16 tlow, thigh;
        uint8  alow, ahigh;
        bool feed_low, feed_high;
    } shift;

    struct OAM {
        uint8 shifts[8], latches[8], counters[8];
    } oam;

public:
    PPU(Bus<PPUBUS_SIZE> *vrambus, Screen *scr)
        : bus(vrambus), screen(scr)
    { }

    void power(bool reset);
    void bus_map(Bus<CPUBUS_SIZE> &bus);
    void on_nmi(auto &&callback)           { nmi_callback = callback; }

    // ppumain.cpp
    void run();

    // these shouldn't be called outside ppumain.cpp
    template <unsigned int Cycle> void ccycle();
    template <unsigned int Line> void lcycle(unsigned int cycle);
    template <unsigned Cycle> void background_cycle();
    void cycle_idle();

private:
    uint8 readreg(const uint16 which);
    void writereg(const uint16 which, const uint8 data);
    uint8 readreg_no_sideeff(const uint16 which) const;

    void output();
    uint8 getcolor(uint8 pal, uint8 palind, bool select);

    void inc_v_horzpos();
    void inc_v_vertpos();
    void copy_v_horzpos();
    void copy_v_vertpos();

    void fetch_nt(bool dofetch);
    void fetch_attr(bool dofetch);
    void fetch_lowbg(bool dofetch);
    void fetch_highbg(bool dofetch);
    void shift_run();
    void shift_fill();
    uint8 bg_output();

    // ppumain.cpp
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

    friend class Debugger::Debugger;
    friend class Debugger::PPUDebugger;
};

} // namespace Core

#endif

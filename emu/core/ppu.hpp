#ifndef CORE_PPU_HPP_INCLUDED
#define CORE_PPU_HPP_INCLUDED

#include <array>
#include <functional>
#include <string>
#include <emu/core/types.hpp>
#include <emu/core/memmap.hpp>
#include <emu/core/bus.hpp>
#include <emu/util/bits.hpp>

namespace Video { class Canvas; }

namespace Core {

class CPU;
class Cartridge;

class PPU {
    Bus *bus;
    Bus *cpubus;
    Video::Canvas *screen;
    uint8 vrammem[VRAM_SIZE];
    uint8 oammem[OAM_SIZE];
    uint8 palmem[PAL_SIZE];
    unsigned long cycles = 0;
    unsigned long lines  = 0;
    std::function<void(void)> nmi_callback;

public:
    struct VRAMAddress {
        uint15 value = 0;

        VRAMAddress() = default;
        VRAMAddress(uint64 x) : value(x) { }
        inline operator uint16() const     { return value; }
        VRAMAddress & operator+=(uint16 n) { value += n; return *this; }

        uint5 coarse_x() const          { return  value &  0x1F; }
        uint5 coarse_y() const          { return (value & (0x1F <<  5)) >>  5; }
        uint2 nt() const                { return (value & (0x03 << 10)) >> 10; }
        uint16 fine_y() const            { return (value & (0x07 << 12)) >> 12; }
        void set_coarse_x(uint5 data)   { value = Util::set_bits(value,  0, 0x1F, data); }
        void set_coarse_y(uint5 data)   { value = Util::set_bits(value,  5, 0x1F, data); }
        void set_nt(uint2 data)         { value = Util::set_bits(value, 10, 0x03, data); }
        void set_fine_y(uint3 data)     { value = Util::set_bits(value, 12, 0x07, data); }
        void switch_nt_horz()           { value ^= 0x400; }
        void switch_nt_vert()           { value ^= 0x800; }
    };

    struct {
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

    bool odd_frame;

    struct {
        VRAMAddress addr;
        VRAMAddress tmp;
        uint8 fine_x;
    } vram;

    struct {
        uint8 nt;
        uint8 attr;
        uint8 low;
        uint8 high;
    } tile;

    struct {
        uint16 tlow, thigh;
        uint8  alow, ahigh;
        bool feed_low, feed_high;
    } shift;

    struct {
        uint8 shifts[8], latches[8], counters[8];
    } oam;

public:
    enum class Mirroring { VERT, HORZ, OTHER };

    void power();
    void reset();
    // ppumain.cpp
    void run();
    std::string get_info();
    void attach_bus(Bus *pb, Bus *cb, Mirroring mirroring);

    void set_mirroring(Mirroring m)        { map_nt(m); }
    void set_screen(Video::Canvas *canvas) { screen = canvas; }
    void set_nmi_callback(auto &&callback) { nmi_callback = callback; }

    // these shouldn't be called outside ppumain.cpp
    template <unsigned int Cycle> void ccycle();
    template <unsigned int Line> void lcycle(unsigned int cycle);
    template <unsigned Cycle> void background_cycle();
    void cycle_idle();

public:
    uint8 readreg(const uint16 which);
    void writereg(const uint16 which, const uint8 data);

    void output();
    uint8 getcolor(bool select, uint8 pal, uint8 palind);
    void map_nt(Mirroring mirroring);

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
};

} // namespace Core

#endif

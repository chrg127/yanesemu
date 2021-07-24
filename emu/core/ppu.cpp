#include "ppu.hpp"

#include <cstdio>
#include <cassert>
#include <functional>
#include <fmt/core.h>
#include <emu/core/bus.hpp>
#include <emu/util/easyrandom.hpp>
#include <emu/util/debug.hpp>

namespace Core {

/* The VRAM address has the following components:
 * FFFNNYYYYYXXXXX
 * then, depending on how you look at it, you can get the X component
 * and the Y component.
 * X : N XXXXX FFF (where N is the first bit of NN and FFF is fine_x)
 * Y : N YYYYY FFF (where N is the second bit of NN and FFF is fine_y) */
static PPU::VRAMAddress inc_v_horzpos(PPU::VRAMAddress addr)
{
    if (addr.coarse_x == 31) {
        addr.coarse_x = 0;
        addr.nt ^= 1;
    } else
        ++addr.coarse_x;
    return addr;
}

static PPU::VRAMAddress inc_v_vertpos(PPU::VRAMAddress addr)
{
    if (addr.fine_y < 7)
        ++addr.fine_y;
    else {
        addr.fine_y = 0;
        uint16 y = addr.coarse_y;
        if (y == 29) {
            y = 0;
            addr.nt ^= 2;
        } else if (y == 31)
            y = 0;
        else
            y += 1;
        addr.coarse_y = y;
    }
    return addr;
}

#define INSIDE_PPU_CPP
#include "ppumain.cpp"
#undef INSIDE_PPU_CPP

void PPU::power(bool reset)
{
    // PPUCTRL
    io.vram_inc     = 0;
    io.sp_pt_addr   = 0;
    io.bg_pt_addr   = 0;
    io.sp_size      = 0;
    io.ext_bus_dir  = 0;
    io.nmi_enabled  = 0;
    // PPUMASK
    io.grey         = 0;
    io.bg_show_left = 0;
    io.sp_show_left = 0;
    io.bg_show      = 0;
    io.sp_show      = 0;
    io.red          = 0;
    io.green        = 0;
    io.blue         = 0;
    // PPUSTATUS
    if (!reset) {
        io.sp_overflow = 1;
        io.sp_zero_hit = 0;
        io.vblank = 1;
    } else {
        io.sp_overflow = 0; // Util::random_between(0, 1);
        io.sp_zero_hit = 0; // Util::random_between(0, 1);
    }
    // OAMADDR
    io.oam_addr = 0;
    // PPUSCROLL and PPUADDR
    if (!reset)
        vram.tmp = 0;
    vram.fine_x = 0;
    io.scroll_latch = 0;
    // PPUDATA
    io.data_buf = 0;
    // other
    odd_frame = 0;
    lines = cycles = 0;
    // i have no idea
    vram.addr = 0;
    vram.tmp = 0;
}

void PPU::bus_map(Bus<CPUBUS_SIZE> &rambus)
{
    rambus.map(PPUREG_START, APU_START,
            [this](uint16 addr)             { return readreg(0x2000 + (addr & 0x7)); },
            [this](uint16 addr, uint8 data) { writereg(0x2000 + (addr & 0x7), data); });
}

uint8 PPU::readreg(const uint16 which)
{
    switch (which) {
    // PPUCTRL, PPUMASK, OAMADDR, PPUSCROLL, PPUADDR
    case 0x2000: case 0x2001: case 0x2003:
    case 0x2005: case 0x2006:
        break;

    // PPUSTATUS
    case 0x2002:
        io.latch |= (io.vblank << 7 | io.sp_zero_hit << 6 | io.sp_overflow << 5);
        io.vblank = 0;
        io.scroll_latch = 0;
        break;

    // OAMDATA
    case 0x2004:
        io.latch = 0;
        break;

    // PPUDATA
    case 0x2007:
        if (vram.addr.v < 0x3F00) {
            io.latch = io.data_buf;
            io.data_buf = bus->read(vram.addr.as_u14());
        } else
            io.latch = bus->read(vram.addr.as_u14());
        vram.addr += (1UL << 5*io.vram_inc);
        break;

#ifdef DEBUG
    default:
        panic("invalid PPU register during read: {:02X}\n", which);
        break;
#endif
    }
    return io.latch;
}

void PPU::writereg(const uint16 which, const uint8 data)
{
    io.latch = data;
    switch (which) {

    // PPUCTRL
    case 0x2000:
        vram.tmp.nt     = data & 0x03;
        io.vram_inc     = data & 0x04;
        io.sp_pt_addr   = data & 0x08;
        io.bg_pt_addr   = data & 0x10;
        io.sp_size      = data & 0x20;
        io.ext_bus_dir  = data & 0x40;
        io.nmi_enabled  = data & 0x80;
        break;

    // PPUMASK
    case 0x2001:
        io.grey          = data & 0x01;
        io.bg_show_left  = data & 0x02;
        io.sp_show_left  = data & 0x04;
        io.bg_show       = data & 0x08;
        io.sp_show       = data & 0x10;
        io.red           = data & 0x20;
        io.green         = data & 0x40;
        io.blue          = data & 0x80;
        break;

    // PPUSTATUS
    case 0x2002:
        break;

    // OAMADDR
    case 0x2003:
        io.oam_addr = data;
        break;

    // OAMDATA
    case 0x2004:
        io.oam_addr++;
        break;

    // PPUSCROLL
    case 0x2005:
        if (!io.scroll_latch) {
            vram.tmp.coarse_x = data >> 3 & 0x1F;
            vram.fine_x       = data & 0x7;
        } else {
            vram.tmp.coarse_y = data >> 3 & 0x1F;
            vram.tmp.fine_y   = data & 0x7;
        }
        io.scroll_latch ^= 1;
        break;

    // PPUADDR
    case 0x2006:
        if (io.scroll_latch == 0) {
            // high byte
            vram.tmp = Util::setbits(vram.tmp.v, 8, 6, data & 0x3F);
            vram.tmp = Util::setbit(vram.tmp.v, 14, 0);
        } else {
            // low byte
            vram.tmp = Util::setbits(vram.tmp.v, 0, 8, data);
            vram.addr = vram.tmp;
        }
        io.scroll_latch ^= 1;
        break;

    // PPUDATA
    case 0x2007:
        bus->write(vram.addr.as_u14(), data);
        vram.addr += (1UL << 5*io.vram_inc);
        break;

#ifdef DEBUG
    default:
        panic("invalid PPU register during write: {:02X}\n", which);
        break;
#endif
    }
}

void PPU::copy_v_horzpos()
{
    vram.addr.coarse_x = vram.tmp.coarse_x;
    vram.addr.nt = Util::setbit(vram.addr.nt, 0, vram.tmp.nt & 1);
}

void PPU::copy_v_vertpos()
{
    vram.addr.coarse_y = vram.tmp.coarse_y;
    vram.addr.fine_y = vram.tmp.fine_y;
    vram.addr.nt = Util::setbit(vram.addr.nt, 1, vram.tmp.nt >> 1 & 1);
}

/* it's probably worth mentioning here that this mask
 * takes all bits of the vram address except for the fine y.
 * this is how the vram address can return to the same tile for 8 lines:
 * in rendering, you can think of the fine y and the rest of the address
 * as separate, where fine y of course indicates the row of the tile. */
uint8 PPU::fetch_nt(uint15 vram_addr)
{
    uint16 addr = 0x2000 | (vram_addr & 0x0FFF);
    return bus->read(addr);
}

/* an attribute address can be composed this way:
 * NN 1111 YYY XXX
 * where YYY/XXX are the highest bits of coarse x/y. */
uint8 PPU::fetch_attr(uint16 nt, uint16 coarse_y, uint16 coarse_x)
{
    uint16 addr = 0x23C0 | nt << 10 | (coarse_y & 0x1C) << 1 | (coarse_x & 0x1C) >> 2;
    return bus->read(addr);
}

/* A pattern table address is formed this way:
 * 0HRRRRCCCCPTTT
 * H - which table is used, left/right. controlled by io.bg_pt_addr.
 * RRRR CCCC - row, column. controlled by the fetch nt byte.
 * P - bit plane. 0 = get the low byte, 1 = get the high byte. (it is implicitly
 * set to 0 here)
 * TTT - fine y, or the current row. fine y is incremented at cycle 256 of each
 * row. */
uint8 PPU::fetch_bg(bool base, uint8 nt, bool bitplane, uint3 fine_y)
{
    uint16 addr = base << 12 | nt << 4 | bitplane << 3 | fine_y;
    return bus->read(addr);
}

void PPU::shift_run()
{
    shift.tile_low  >>= 1;
    shift.tile_high >>= 1;
    shift.attr_low  >>= 1;
    shift.attr_high >>= 1;
    shift.attr_low  = Util::setbit(shift.attr_low,  7, shift.feed_low );
    shift.attr_high = Util::setbit(shift.attr_high, 7, shift.feed_high);
}

void PPU::shift_fill()
{
    // reversing the bits here fixes the bug with reversed tiles.
    // however, i'm pretty sure this is not the solution.
    shift.tile_low  = Util::setbits(shift.tile_low,  8, 8, Util::reverse_bits(tile.low));
    shift.tile_high = Util::setbits(shift.tile_high, 8, 8, Util::reverse_bits(tile.high));
    // shift.tlow  = Util::setbits(shift.tlow,  8, 8, tile.low);
    // shift.thigh = Util::setbits(shift.thigh, 8, 8, tile.high);

    // these are, respectively, bit 1 and 6 of vram.addr
    uint8 bit1 = Util::getbit(vram.addr.coarse_x, 1);
    uint8 bit2 = Util::getbit(vram.addr.coarse_y, 1);
    // (00,01,10,11) -> (0,2,4,6)
    int bitno = (bit2 << 1 | bit1) * 2;

    uint2 bits = Util::getbits(tile.attr, bitno, 2);
    shift.feed_high = bits & 1;
    shift.feed_low  = (bits >> 1) & 1;
}

uint8 PPU::bg_output()
{
    uint8 mask = 1UL << vram.fine_x;
    bool hi    = shift.tile_high & mask;
    bool low   = shift.tile_low  & mask;
    bool at1   = shift.attr_high & mask;
    bool at2   = shift.attr_low  & mask;
    return getcolor(at1 << 1 | at2, hi << 1 | low, /* background */ 0);
}

/* SIIPP
 *    ^^ pixel value (palind)
 *  ^^ palette number (pal)
 * ^ sprite or background (select)
 */
uint8 PPU::getcolor(uint2 pal, uint2 palind, bool select)
{
    uint5 n = select << 4 | pal << 2 | palind;
    return bus->read(0x3F00 + n);
}

void PPU::output()
{
    uint8 bgpixel = io.bg_show ? bg_output() : 0;
    auto x = cycles % PPU_MAX_LCYCLE;
    auto y = lines % PPU_MAX_LINES;
    assert((y <= 239 || y == 261) && x <= 256);
    // is there any fucking document that says when i have to output pixels
    // and doesn't have a shitty explanation?
    if (x == 256)
        return;
    if (y == 261)
        return;
    screen->output(x, y, bgpixel);
}

} // namespace Core


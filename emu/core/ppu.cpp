#include "ppu.hpp"

#include <cstdio>
#include <cassert>
#include <functional>
#include <fmt/core.h>
#include <emu/core/bus.hpp>
#include <emu/util/easyrandom.hpp>
#include <emu/util/debug.hpp>

namespace Core {

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
        if (vram.addr < 0x3F00) {
            io.latch = io.data_buf;
            io.data_buf = bus->read(vram.addr);
        } else
            io.latch = bus->read(vram.addr);
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
            vram.tmp = Util::setbits(vram.tmp, 8, 6, data & 0x3F);
            vram.tmp = Util::setbit(vram.tmp, 14, 0);
        } else {
            // low byte
            vram.tmp = Util::setbits(vram.tmp, 0, 8, data);
            vram.addr = vram.tmp;
        }
        io.scroll_latch ^= 1;
        break;

    // PPUDATA
    case 0x2007:
        bus->write(vram.addr, data);
        vram.addr += (1UL << 5*io.vram_inc);
        break;

#ifdef DEBUG
    default:
        panic("invalid PPU register during write: {:02X}\n", which);
        break;
#endif
    }
}

uint8 PPU::readreg_no_sideeff(const uint16 which) const
{
    switch (which) {
    case 0x2000: return vram.tmp.nt
                      | io.vram_inc    << 2
                      | io.sp_pt_addr  << 3
                      | io.bg_pt_addr  << 4
                      | io.sp_size     << 5
                      | io.ext_bus_dir << 6
                      | io.nmi_enabled << 7;
    case 0x2001: return io.grey
                      | io.bg_show_left << 1
                      | io.sp_show_left << 2
                      | io.bg_show      << 3
                      | io.sp_show      << 4
                      | io.red          << 5
                      | io.green        << 6
                      | io.blue         << 7;
    case 0x2002: return io.vblank << 7
                      | io.sp_zero_hit << 6
                      | io.sp_overflow << 5;
    case 0x2003: return io.oam_addr;
    case 0x2004: return 0;
    case 0x2005: return !io.scroll_latch ? vram.fine_x     << 5 | vram.tmp.coarse_x
                                         : vram.tmp.fine_y << 5 | vram.tmp.coarse_y;
    case 0x2006: return !io.scroll_latch ? vram.tmp & 0xFF : vram.tmp >> 8 & 0xFF;
    case 0x2007: return vram.addr < 0x3F00 ? io.data_buf
                                           : bus->read(vram.addr);
    default: return 0xFF;
    }
}

void PPU::output()
{
    uint8 bgpixel = bg_output();
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

/* The VRAM address has the following components:
 * FFFNNYYYYYXXXXX
 * then, depending on how you look at it, you can get the X component
 * and the Y component.
 * X : N XXXXX FFF (where N is the first bit of NN and FFF is fine_x)
 * Y : N YYYYY FFF (where N is the second bit of NN and FFF is fine_y) */
void PPU::inc_v_horzpos()
{
    if (!io.bg_show)
        return;
    if (vram.addr.coarse_x == 31) {
        vram.addr.coarse_x = 0;
        vram.addr.nt ^= 1;
    } else
        ++vram.addr.coarse_x;
}

void PPU::inc_v_vertpos()
{
    if (!io.bg_show)
        return;
    if (vram.addr.fine_y < 7)
        ++vram.addr.fine_y;
    else {
        vram.addr.fine_y = 0;
        uint16 y = vram.addr.coarse_y;
        if (y == 29) {
            y = 0;
            vram.addr.nt ^= 2;
        } else if (y == 31)
            y = 0;
        else
            y += 1;
        vram.addr.coarse_y = y;
    }
}

void PPU::copy_v_horzpos()
{
    if (!io.bg_show)
        return;
    vram.addr.coarse_x = vram.tmp.coarse_x;
    vram.addr.nt = Util::setbit(vram.addr.nt, 0, vram.tmp.nt & 1);
}

void PPU::copy_v_vertpos()
{
    if (!io.bg_show)
        return;
    vram.addr.coarse_y = vram.tmp.coarse_y;
    vram.addr.fine_y = vram.tmp.fine_y;
    vram.addr.nt = Util::setbit(vram.addr.nt, 1, vram.tmp.nt >> 1 & 1);
}

void PPU::fetch_nt(bool dofetch)
{
    if (!dofetch)
        return;
    /* it's probably worth mentioning here that this mask
     * takes all bits of the vram address except for the fine y.
     * this is how the vram address can return to the same tile for 8 lines:
     * in rendering, you can think of the fine y and the rest of the address
     * as separate, where fine y of course indicates the row of the tile. */
    tile.nt = bus->read(0x2000 | (vram.addr & 0x0FFF));
}

void PPU::fetch_attr(bool dofetch)
{
    // an attribute address can be composed this way:
    // NN 1111 YYY XXX
    // where YYY/XXX are the highest bits of coarse x/y.
    if (!dofetch)
        return;
    tile.attr = bus->read(0x23C0
                       |  uint16(vram.addr.nt) << 10
                       | (uint16(vram.addr.coarse_y) & 0x1C) << 1
                       | (uint16(vram.addr.coarse_x) & 0x1C) >> 2);
}

/* A pattern table address is formed this way:
 * 0HRRRRCCCCPTTT
 * H - which table is used, left/right. controlled by io.bg_pt_addr.
 * RRRR CCCC - row, column. controlled by the fetch nt byte.
 * P - bit plane. 0 = get the low byte, 1 = get the high byte.
 * TTT - fine y, or the current row. fine y is incremented at cycle 256 of each
 * row. */
void PPU::fetch_lowbg(bool dofetch)
{
    if (!dofetch)
        return;
    uint16 lowbg_addr = (io.bg_pt_addr << 12)
                      | (tile.nt       << 4)
                      | vram.addr.fine_y;
    tile.low = bus->read(lowbg_addr);
}

void PPU::fetch_highbg(bool dofetch)
{
    if (!dofetch)
        return;
    uint16 highbg_addr = (io.bg_pt_addr << 12)
                       | (tile.nt       << 4)
                       | (1UL           << 3) // or otherwise... add 8
                       | vram.addr.fine_y;
    tile.high = bus->read(highbg_addr);
}

void PPU::shift_run()
{
    if (!io.bg_show)
        return;
    shift.tlow  >>= 1;
    shift.thigh >>= 1;
    shift.ahigh >>= 1;
    shift.alow  >>= 1;
    shift.ahigh = Util::setbit(shift.ahigh, 7, shift.feed_high);
    shift.ahigh = Util::setbit(shift.alow,  7, shift.feed_low );
}

void PPU::shift_fill()
{
    if (!io.bg_show)
        return;
    shift.tlow  = Util::setbits(shift.tlow,  8, 8, tile.low);
    shift.thigh = Util::setbits(shift.thigh, 8, 8, tile.high);
    // TODO: this is definitely wrong
    uint16 v = vram.addr;
    uint8 attr_mask = 0b11 << (~((v >> 1 & 1) | (v >> 6 & 1)))*2;
    shift.feed_high = tile.attr & attr_mask;
    shift.feed_low  = tile.attr & attr_mask;
}

// pal: 0-3, one of the 4 defined palettes for this frame
// palind: which color of the palette
// select: select if it's a background palette or a sprite palette
// first 2 values are 2 bits big, not 8
uint8 PPU::getcolor(uint8 pal, uint8 palind, bool select)
{
    // this is a 5 bit number
    uint8 n = select << 4 | pal << 2 | palind;
    return bus->read(0x3F00 + n);
}

uint8 PPU::bg_output()
{
    if (!io.bg_show)
        return 0;
    uint8 mask      = 1UL << vram.fine_x;
    bool lowbit     = shift.tlow  & mask;
    bool hibit      = shift.thigh & mask;
    bool at1        = shift.ahigh & mask;
    bool at2        = shift.alow  & mask;
    uint8 pal       = at1   << 1 | at2;
    uint8 palind    = hibit << 1 | lowbit;
    return getcolor(pal, palind, 0);
}

} // namespace Core


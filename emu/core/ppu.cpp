#include "ppu.hpp"

#include <cstdio>
#include <cassert>
#include <functional>
#include <fmt/core.h>
#include <emu/core/bus.hpp>
#include <emu/util/easyrandom.hpp>
#include <emu/util/debug.hpp>

namespace core {

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
        u16 y = addr.coarse_y;
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
    // OAMAddr
    oam.addr = 0;
    // PPUScroll and PPUAddr
    if (!reset)
        vram.tmp = 0;
    vram.fine_x = 0;
    io.scroll_latch = 0;
    // PPUData
    io.data_buf = 0;
    // other
    odd_frame = 0;
    lines = cycles = 0;
    vram.addr = 0;
    vram.tmp = 0;
    std::fill(oam.mem.begin(), oam.mem.end(), 0);
    std::fill(secondary_oam.mem.begin(), secondary_oam.mem.end(), 0);
}

u8 PPU::readreg(u16 addr)
{
    switch (addr) {
    // PPUCTRL,       PPUMASK,     OAMAddr,     PPUScroll,   PPUAddr
    case 0x2000: case 0x2001: case 0x2003: case 0x2005: case 0x2006:
        break;

    // PPUSTATUS
    case 0x2002:
        io.latch |= (io.vblank << 7 | io.sp_zero_hit << 6 | io.sp_overflow << 5);
        io.vblank = 0;
        io.scroll_latch = 0;
        break;

    // OAMDATA
    case 0x2004:
        io.latch = oam.mem[oam.addr];
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
        panic("invalid PPU register during read: {:02X}\n", addr);
        break;
#endif
    }
    return io.latch;
}

void PPU::writereg(u16 addr, u8 data)
{
    io.latch = data;
    switch (addr) {

    // PPUCTRL
    case 0x2000:
        vram.tmp.nt     = data & 0x03;
        io.vram_inc     = data & 0x04;
        io.sp_pt_addr   = data & 0x08;
        io.bg_pt_addr   = data & 0x10;
        io.sp_size      = (data & 0x20) >> 5;
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
        oam.addr = data;
        break;

    // OAMDATA
    case 0x2004:
        if (lines > 239 && lines < 261)
            oam.mem[oam.addr++] = data;
        break;

    // PPUSCROLL
    case 0x2005:
        if (!io.scroll_latch) {
            vram.tmp.coarse_x = bits::getbits(data, 3, 5);
            vram.fine_x       = bits::getbits(data, 0, 3);
        } else {
            vram.tmp.coarse_y = bits::getbits(data, 3, 5);
            vram.tmp.fine_y   = bits::getbits(data, 0, 3);
        }
        io.scroll_latch ^= 1;
        break;

    // PPUADDR
    case 0x2006:
        if (io.scroll_latch == 0)
            vram.tmp.v = bits::setbits(vram.tmp.v, 8, 8, data & 0x3F);
        else {
            vram.tmp = bits::setbits(vram.tmp.v, 0, 8, data);
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
        panic("invalid PPU register during write: {:02X}\n", addr);
        break;
#endif
    }
}

void PPU::copy_v_horzpos()
{
    vram.addr.coarse_x = vram.tmp.coarse_x;
    vram.addr.nt = bits::setbit(vram.addr.nt, 0, vram.tmp.nt & 1);
}

void PPU::copy_v_vertpos()
{
    vram.addr.coarse_y = vram.tmp.coarse_y;
    vram.addr.fine_y = vram.tmp.fine_y;
    vram.addr.nt = bits::setbit(vram.addr.nt, 1, vram.tmp.nt >> 1 & 1);
}

/*
 * it's probably worth mentioning here that this mask
 * takes all bits of the vram address except for the fine y.
 * this is how the vram address can return to the same tile for 8 lines:
 * in rendering, you can think of the fine y and the rest of the address
 * as separate, where fine y of course indicates the row of the tile.
 */
u8 PPU::fetch_nt(u15 vram_addr)
{
    u16 addr = 0x2000 | (vram_addr & 0x0FFF);
    return bus->read(addr);
}

/*
 * an attribute address can be composed this way:
 * NN 1111 YYY XXX
 * where YYY/XXX are the highest bits of coarse x/y.
 */
u8 PPU::fetch_attr(u16 nt, u16 coarse_y, u16 coarse_x)
{
    u16 addr = 0x23C0 | nt << 10 | (coarse_y & 0x1C) << 1 | (coarse_x & 0x1C) >> 2;
    return bus->read(addr);
}

/*
 * A pattern table address is formed this way:
 * 0HRRRRCCCCPTTT
 * H - which table is used, left or right.
 * RRRR CCCC - row, column. Imagine the pattern table as a grid, without
 * thinking about the real representation.
 * P - bit plane. 0 = get the low byte, 1 = get the high byte.
 * TTT - row. A tile is composed of 8 rows, these 3 bits decide which one.
 */
u8 PPU::fetch_pt(bool base, u8 nt, bool bitplane, u3 fine_y)
{
    u16 addr = base << 12 | nt << 4 | bitplane << 3 | fine_y;
    return bus->read(addr);
}

// This function checks for sprite size and row before fetching from the pattern table
u8 PPU::fetch_pt_sprite(bool sp_size, u8 nt, bool bitplane, unsigned row)
{
    if (row > 16)
        return 0;
    if (!sp_size)
        return fetch_pt(io.sp_pt_addr, nt, bitplane, row);
    unsigned bit = bits::getbit(row, 3);
    return fetch_pt(nt & 1, (nt & 0xFE) + bit, bitplane, row - 8*bit);
}

void PPU::background_shift_run()
{
    shift.pt_low    <<= 1;
    shift.pt_high   <<= 1;
    shift.attr_low  <<= 1;
    shift.attr_high <<= 1;
    shift.attr_low  = bits::setbit(shift.attr_low,  0, shift.feed_low );
    shift.attr_high = bits::setbit(shift.attr_high, 0, shift.feed_high);
}

void PPU::background_shift_fill()
{
    shift.pt_low  = bits::setbits(shift.pt_low,  0, 8, tile.pt_low);
    shift.pt_high = bits::setbits(shift.pt_high, 0, 8, tile.pt_high);

    // these are, respectively, bit 1 and 6 of vram.addr
    unsigned bit1 = bits::getbit(vram.addr.coarse_x, 1);
    unsigned bit2 = bits::getbit(vram.addr.coarse_y, 1);
    // (00,01,10,11) -> (0,2,4,6)
    unsigned bitno = (bit2 << 1 | bit1) << 1;
    u2 palette_num = bits::getbits(tile.attr, bitno, 2);
    shift.feed_high = bits::getbit(palette_num, 1);
    shift.feed_low  = bits::getbit(palette_num, 0);
}

// fine x indicates which bit we want to get. we save the mask since it'll be
// used for 4 times.
// the mask is shifted for both pt_high and pt_low because for those two we must
// look at the high byte.
std::pair<u2, u2> PPU::background_output()
{
    if (!io.bg_show)
        return std::make_pair(0, 0);
    unsigned mask = 0x80 >> vram.fine_x;
    bool hi    = shift.pt_high & mask << 8;
    bool low   = shift.pt_low  & mask << 8;
    bool athi  = shift.attr_high & mask;
    bool atlow = shift.attr_low  & mask;
    return std::make_pair(athi << 1 | atlow, hi << 1 | low);
}

void PPU::sprite_update_flags(unsigned line)
{
    if (oam.sp_counter == 0)
        oam.inrange = (line - oam.data) < unsigned(8 + io.sp_size * 8);
    if (oam.inrange && !oam.addr_overflow) {
        // determine if this is a sprite 0 hit
        if (!io.sp_zero_hit && oam.addr == 0)
            oam.sp0_next = true;
        if (!io.sp_overflow && secondary_oam.full())
            io.sp_overflow = true;
        oam.inc();
        secondary_oam.inc();
    } else {
        oam.inc(); oam.inc(); oam.inc(); oam.inc();
        if (secondary_oam.full() && !oam.addr_overflow)
            oam.inc();
    }
    oam.addr_overflow = oam.addr == 0;
}

void PPU::sprite_shift_run()
{
    for (int i = 0; i < 8; i++) {
        if (oam.xpos[i] != 0)
            oam.xpos[i]--;
        else {
            oam.pt_low[i]  <<= 1;
            oam.pt_high[i] <<= 1;
        }
    }
}

std::tuple<u2, u2, u8> PPU::sprite_output(unsigned x)
{
    if (!io.sp_show)
        return std::make_tuple(0, 0, 0);
    for (u8 i = 0; i < 8; i++) {
        if (oam.xpos[i] != 0)
            continue;
        bool low  = bits::getbit(oam.pt_low[i],  7);
        bool high = bits::getbit(oam.pt_high[i], 7);
        if (low != 0 || high != 0)
            return std::make_tuple(
                bits::getbits(oam.attrs[i], 0, 2),
                high << 1 | low,
                i
            );
    }
    return std::make_tuple(0, 0, 0);
}

/*
 * SIIPP
 *    ^^ pixel value (ind) (also known as whatever we get from the pattern table)
 *  ^^ palette number (row) (attributes in tiles and sprites)
 * ^ sprite or background (select)
 *
 * As for terminology:
 * palette row = which palette row an object (tile/sprite) uses. For example, a
 * tile that uses palette row 2 will uses one of the colors from $3F09-$3F0B.
 * palette index = which of the 3 colors (+ trasparency) an object uses. A tile
 * with palette row 2 and palette index 1 will use the color at $3F09.
 * Furthermore, any object with palette index 0 will automatically use the color
 * from $3F00 regardless of palette row.
 */
u8 PPU::output(unsigned x)
{
    auto [bg_row, bg_ind]         = background_output();
    auto [sp_row, sp_ind, sp_num] = sprite_output(x);

    auto getcolor = [this](u2 row, u2 ind, bool select) -> u8
    {
        u5 n = select << 4 | row << 2 | ind;
        return bus->read(0x3F00 + n);
    };

    int n = (bg_ind != 0) << 1 | (sp_ind != 0);
    switch (n) {
    case 0: default: return bus->read(0x3F00);
    case 1: return getcolor(sp_row, sp_ind, 1);
    case 2: return getcolor(bg_row, bg_ind, 0);
    case 3: {
        // check for sprite 0 hit.
        if (!io.sp_zero_hit && oam.sp0_curr && sp_num == 0) {// && !(!io.sp_show_left && x <= 7))
            io.sp_zero_hit = true;
            oam.sp0_curr = 0;
        }
        bool priority = bits::getbit(oam.attrs[sp_num], 5);
        return priority ? getcolor(bg_row, bg_ind, 0)
                        : getcolor(sp_row, sp_ind, 1);
    }
    }
}

void PPU::render()
{
    auto x = cycles;
    auto y = lines;
    u8 pixel = output(x);
    screen->output(x-1, y, pixel);
}

void PPU::vblank_begin()
{
    io.vblank = 1;
    nmi_callback(io.nmi_enabled);
}

void PPU::vblank_end()
{
    io.vblank      = 0;
    io.sp_zero_hit = 0;
    io.sp_overflow = 0;
}

} // namespace core

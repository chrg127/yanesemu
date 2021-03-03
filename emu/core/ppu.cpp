#include <emu/core/ppu.hpp>

#include <cstdio>
#include <cassert>
#include <functional>
#include <fmt/core.h>
#include <emu/core/cpu.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/util/file.hpp>
#include <emu/util/easyrandom.hpp>
#include <emu/util/debug.hpp>
#include <emu/video/video.hpp>

namespace Core {

#define INSIDE_PPU_CPP
#include "ppumain.cpp"
#undef INSIDE_PPU_CPP

void PPU::power()
{
    vram.addr = 0;
    vram.tmp = 0;
    // PPUCTRL
    io.vram_inc = 0;
    io.sp_pt_addr = 0;
    io.bg_pt_addr = 0;
    io.sp_size = 0;
    io.ext_bus_dir = 0;
    io.nmi_enabled = 0;
    // PPUMASK
    io.grey = 0;
    io.bg_show_left = 0;
    io.sp_show_left = 0;
    io.bg_show = 0;
    io.sp_show = 0;
    io.red = 0;
    io.green = 0;
    io.blue = 0;
    // PPUSTATUS
    io.sp_overflow = 1;
    io.sp_zero_hit = 0;
    io.vblank = 1;
    // OAMADDR
    io.oam_addr = 0;
    // PPUSCROLL and PPUADDR
    vram.tmp = 0;
    vram.fine_x = 0;
    io.scroll_latch = 0;
    // PPUDATA
    io.data_buf = 0;
    // other
    odd_frame = 0;
    lines = cycles = 0;
    // randomize memory
    // for (auto &cell : oammem)
    //     cell = Util::random8();
    // for (uint16 i = PAL_START; i < 0x3F20; i++)
    //     bus->write(i, Util::random8());
    // for (uint16 i = NT_START; i < 0x3000; i++)
    //     bus->write(i, Util::random8());
    // we should also randomize chr-ram
}

void PPU::reset()
{
    // // PPUCTRL
    // bg.pt_addr = oam.pt_addr = ext_bus_dir = nmi_enabled = 0;
    // vram.inc = 1; // if 0, inc = 1. if 1, inc = 32
    // // PPUMASK
    // bg.show_leftmost  = bg.show  = 0;
    // oam.show_leftmost = oam.show = 0;
    // effects.grey  = 0;
    // effects.red   = 0;
    // effects.green = 0;
    // effects.red   = 0;
    // // PPUSTATUS
    // spr_ov = Util::random_between(0, 1);
    // spr0hit = Util::random_between(0, 1);
    // // vblank = unchanged;
    // // OAMADDR
    // // oam.addr = unchanged;
    // // PPUSCROLL and PPUADDR
    // vram.toggle = 0;
    // // vram.v = 0;
    // // vram.t = unchanged;
    // vram.fine_x = 0;
    // // PPUDATA
    // vram.readbuf = 0;

    // odd_frame = 0;
    // lines = 0;
    // cycles = 0;
    // for (auto &cell : oammem)
    //     cell = Util::random8();
    // // palette, nt ram and chr-ram is unchanged
}

std::string PPU::get_info()
{
    return fmt::format("line = {:03}; cycle = {:03}; v = {:02X}", lines%262, cycles%341, vram.addr.value.value());
}

void PPU::attach_bus(Bus *pb, Bus *cb, Mirroring mirroring)
{
    bus = pb;
    cpubus = cb;
    cpubus->map(PPUREG_START, APU_START,
            [=](uint16 addr)             { return readreg(0x2000 + (addr & 0x7)); },
            [=](uint16 addr, uint8 data) { writereg(0x2000 + (addr & 0x7), data); });
    bus->map(PAL_START, 0x4000,
            [=](uint16 addr)             { return palmem[addr & 0x1F]; },
            [=](uint16 addr, uint8 data) { palmem[addr & 0x1F] = data; });
    map_nt(mirroring);
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
        assert(false);
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
        vram.tmp.set_nt(data & 0x03);
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
            vram.tmp.set_coarse_x(data & (0x1F << 3));
            vram.fine_x = data & 0x7;
        } else {
            vram.tmp.set_coarse_y(data & (0x1F << 3));
            vram.tmp.set_fine_y(data & 0x7);
        }
        io.scroll_latch ^= 1;
        break;

    // PPUADDR
    case 0x2006:
        if (io.scroll_latch == 0) {
            vram.tmp = Util::set_bits(vram.tmp, 8, 0x3F, data & 0x3F);
            vram.tmp = Util::set_bit(vram.tmp, 14, 0);
        } else {
            vram.tmp = Util::set_bits(vram.tmp, 0, 0xFF, data);
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
        assert(false);
        break;
#endif
    }
}

void PPU::output()
{
    const uint8 bgpixel = bg_output();
    uint32 color = 0;
    if (bgpixel == 232)
        color = 0xFF0000FF;
    else if (bgpixel == 214)
        color = 0x00FF00FF;
    else
        color = 0x0000FFFF;
    const auto x = cycles % 341;
    const auto y = lines % 262;
    assert((y <= 239 || y == 261) && x <= 256);
    // is there any fucking document that says when i have to output pixels
    // and doesn't have a shitty explanation?
    if (x == 256)
        return;
    if (y == 261)
        return;
    screen->drawpixel(x, y, color);
}

// palette: 0-3, one of the 4 defined palettes for this frame
// color_index: which color of the palette
// bg_or_sp: select if it's a background palette or a sprite palette
// first 2 values are 2 bits big, not 8
uint8 PPU::getcolor(bool select, uint8 pal, uint8 palind)
{
    // this is a 5 bit number
    uint8 n = select << 4 | pal << 2 | palind;
    return bus->read(0x3F00 + n);
}

void PPU::map_nt(Mirroring mirroring)
{
    unsigned mask = 0;
    switch (mirroring) {
    case Mirroring::VERT: mask = 0x7FF; break;
    case Mirroring::HORZ: mask = 0xBFF; break;
    default: assert(false);
    }
    bus->map(NT_START, PAL_START,
            [=](uint16 addr)             { return vrammem[(addr & mask)]; },
            [=](uint16 addr, uint8 data) { vrammem[(addr & mask)] = data; });
}

void PPU::inc_v_horzpos()
{
    if (!io.bg_show)
        return;
    if (vram.addr.coarse_x() == 31) {
        vram.addr.set_coarse_x(0);
        vram.addr.switch_nt_horz();
    } else
        vram.addr += 1;
}

void PPU::inc_v_vertpos()
{
    if (!io.bg_show)
        return;
    if (vram.addr.fine_y() < 7)
        vram.addr.set_fine_y(vram.addr.fine_y() + 1);
    else {
        vram.addr.set_fine_y(0);
        auto y = vram.addr.coarse_y();
        if (y == 29) {
            y = 0;
            vram.addr.switch_nt_vert();
        } else if (y == 31)
            y = 0;
        else
            y += 1;
        vram.addr.set_coarse_y(y);
    }
}

void PPU::copy_v_horzpos()
{
    if (!io.bg_show)
        return;
    vram.addr.set_coarse_x(vram.tmp.coarse_x());
    vram.addr = Util::set_bit(vram.addr, 10, vram.tmp.nt() & 1);
}

void PPU::copy_v_vertpos()
{
    if (!io.bg_show)
        return;
    vram.addr.set_coarse_y(vram.tmp.coarse_y());
    vram.addr = Util::set_bit(vram.addr, 11, (vram.tmp.nt() & 2) >> 1);
    vram.addr.set_fine_y(vram.tmp.fine_y());
}

void PPU::fetch_nt(bool dofetch)
{
    if (!dofetch)
        return;
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
                       |  uint16(vram.addr.nt()) << 10
                       | (uint16(vram.addr.coarse_y()) & 0x1C << 1)
                       | (uint16(vram.addr.coarse_x()) & 0x1C >> 2));
}

void PPU::fetch_lowbg(bool dofetch)
{
    if (!dofetch)
        return;
    tile.low = bus->read(0x1000 * io.bg_pt_addr + tile.nt);
}

void PPU::fetch_highbg(bool dofetch)
{
    if (!dofetch)
        return;
    tile.high = bus->read(0x1000 * io.bg_pt_addr + tile.nt + 8);
}

void PPU::shift_run()
{
    shift.tlow    >>= 1;
    shift.thigh   >>= 1;
    shift.ahigh >>= 1;
    shift.alow  >>= 1;
    shift.ahigh = Util::set_bit(shift.ahigh, 7, shift.feed_high);
    shift.ahigh = Util::set_bit(shift.alow,  7, shift.feed_low );
}

void PPU::shift_fill()
{
    shift.tlow  = Util::set_bits(shift.tlow,  8, 0xFF, tile.low);
    shift.thigh = Util::set_bits(shift.thigh, 8, 0xFF, tile.high);
    // TODO: this is definitely fucking wrong
    uint16 v = vram.addr;
    uint8 attr_mask = 0b11 << (~((v >> 1 & 1) | (v >> 6 & 1)))*2;
    shift.feed_high = tile.attr & attr_mask;
    shift.feed_low  = tile.attr & attr_mask;
}

uint8 PPU::bg_output()
{
    uint8 mask      = 1UL << vram.fine_x;
    bool lowbit     = shift.tlow  & mask;
    bool hibit      = shift.thigh & mask;
    bool at1        = shift.ahigh & mask;
    bool at2        = shift.alow  & mask;
    uint8 pal       = at1   << 1 | at2;
    uint8 palind    = hibit << 1 | lowbit;
    return getcolor(0, pal, palind);
}

} // namespace Core


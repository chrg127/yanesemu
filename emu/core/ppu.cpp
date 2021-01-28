#include <emu/core/ppu.hpp>

#include <cstdio>
#include <cassert>
// #include <cstring> // memset
#include <functional>
#include <fmt/core.h>
#include <emu/core/cpu.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/util/file.hpp>
#include <emu/util/easyrandom.hpp>
#define DEBUG
#include <emu/util/debug.hpp>

namespace Core {

#define INSIDE_PPU_CPP
#include "ppumain.cpp"
#undef INSIDE_PPU_CPP

void PPU::power()
{
    // PPUCTRL
    bg.nt_addr = bg.pt_addr = oam.pt_addr = ext_bus_dir = nmi_enabled = 0;
    vram.inc = 1; // if 0, inc = 1. if 1, inc = 32
    // PPUMASK
    bg.show_leftmost  = bg.show  = 0;
    oam.show_leftmost = oam.show = 0;
    effects.grey  = 0;
    effects.red   = 0;
    effects.green = 0;
    effects.red   = 0;
    // PPUSTATUS
    spr_ov = spr0hit = vblank = 0;
    // OAMADDR
    oam.addr = 0;
    // PPUSCROLL and PPUADDR
    // vram.v = 0;
    vram.toggle = 0;
    vram.t      = 0;
    vram.fine_x = 0;
    // PPUDATA
    vram.readbuf = 0;
    // other stuff
    odd_frame = 0;
    lines = cycles = 0;
    // randomize memory
    for (auto &cell : oam.oammem)
        cell = Util::random8();
    for (uint16 i = PAL_START; i < 0x3F20; i++)
        bus->write(i, Util::random8());
    for (uint16 i = NT_START; i < 0x3000; i++)
        bus->write(i, Util::random8());
    // we should also randomize chr-ram
}

void PPU::reset()
{
    // PPUCTRL
    bg.nt_addr = bg.pt_addr = oam.pt_addr = ext_bus_dir = nmi_enabled = 0;
    vram.inc = 1; // if 0, inc = 1. if 1, inc = 32
    // PPUMASK
    bg.show_leftmost  = bg.show  = 0;
    oam.show_leftmost = oam.show = 0;
    effects.grey  = 0;
    effects.red   = 0;
    effects.green = 0;
    effects.red   = 0;
    // PPUSTATUS
    spr_ov = Util::random_between(0, 1);
    spr0hit = Util::random_between(0, 1);
    // vblank = unchanged;
    // OAMADDR
    // oam.addr = unchanged;
    // PPUSCROLL and PPUADDR
    vram.toggle = 0;
    // vram.v = 0;
    // vram.t = unchanged;
    vram.fine_x = 0;
    // PPUDATA
    vram.readbuf = 0;

    odd_frame = 0;
    lines = 0;
    cycles = 0;
    for (auto &cell : oam.oammem)
        cell = Util::random8();
    // palette, nt ram and chr-ram is unchanged
}

uint8 PPU::readreg(const uint16 which)
{
    switch (which) {
    case 0x2000: case 0x2001: case 0x2003:
    case 0x2005: case 0x2006: case 0x4014:
        return io_latch;

    case 0x2002:
        io_latch |= (vblank << 7 | spr0hit << 6 | spr_ov << 5);
        vblank = 0;
        vram.toggle = 0;
        return io_latch;

    case 0x2004:
        io_latch = 0;
        // io_latch = oam.data;
        return io_latch;

    case 0x2007:
        if (vram.v < 0x3F00) {
            io_latch = vram.readbuf;
            vram.readbuf = bus->read(vram.v);
        } else
            io_latch = bus->read(vram.v);
        vram.v += vram.inc;
        return io_latch;

#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
    // unreachable
    return io_latch;
}

void PPU::writereg(const uint16 which, const uint8 data)
{
    io_latch = data;
    switch (which) {
    case 0x2000:
        nmi_enabled = data & 0x80;
        ext_bus_dir = data & 0x40;
        vram.inc    = (data & 0x04) ? 32 : 1;
        oam.sprsize = data & 0x20;
        bg.pt_addr  = data & 0x10;
        oam.pt_addr = data & 0x08;
        bg.nt_addr  = data & 0x03;
        vram.t      = (data & 0x03) | (vram.t & 0xF3FF);
        break;

    case 0x2001:
        effects.grey      = data & 0x01;
        bg.show_leftmost  = data & 0x02;
        oam.show_leftmost = data & 0x04;
        bg.show           = data & 0x08;
        oam.show          = data & 0x10;
        effects.red       = data & 0x20;
        effects.green     = data & 0x40;
        effects.blue      = data & 0x80;
        break;

    case 0x2002:
        break;

    case 0x2003:
        oam.addr = data;
        break;

    case 0x2004:
        // this should probably write to oam, not to some register
        // oam.data = data;
        oam.addr++;
        break;

    case 0x2005:
        if (vram.toggle == 0) {
            vram.t = (data &  0xF8) >> 3  | (vram.t & ~(0xF8 >> 3));
            vram.fine_x = data & 0x7;
        } else {
            vram.t = (data &  0x07) << 12 | (vram.t & ~(0x07 << 12));
            vram.t = (data & ~0xF8) << 2  | (vram.t & ~(0xF8 << 2 ));
        }
        vram.toggle ^= 1;
        break;

    case 0x2006:
        if (vram.toggle == 0) {
            vram.t = (data & 0x3F) << 8 | (vram.t & 0xFF);
        } else {
            vram.t = data | (vram.t & 0xFF00) ;
            vram.v = vram.t;
        }
        vram.toggle ^= 1;
        break;

    case 0x2007:
        bus->write(vram.v, data);
        vram.v += vram.inc;
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
    // uint8 bgpixel = bg.output();
    // assert(lines <= 256 && cycles <= 239);
    // screen[lines*239+cycles] = bgpixel;
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

std::string PPU::get_info()
{
    return fmt::format("line = {:03}; cycle = {:03}; v = {:02X}", lines%262, cycles%341, vram.v);
}

void PPU::attach_bus(Bus *pb, Bus *cb)
{
    std::function<uint16(uint16)> address_nametab;

    bus = pb;
    cpubus = cb;
    switch (mirroring) {
    case 0: address_nametab = [](uint16 x) { return x &= ~0x800; }; break;
    case 1: address_nametab = [](uint16 x) { return x &= ~0x400; }; break;
    default: assert(false);
    }
    cpubus->map(PPUREG_START, APU_START,
            [=](uint16 addr)             { return readreg(addr); },
            [=](uint16 addr, uint8 data) { writereg(addr, data); });
    bus->map(NT_START, PAL_START,
            [=](uint16 addr)             { return vram.mem[address_nametab(addr)]; },
            [=](uint16 addr, uint8 data) { vram.mem[address_nametab(addr)] = data; });
    bus->map(PAL_START, VRAM_SIZE,
            [=](uint16 addr)             { return vram.mem[addr & ~0xE0]; },
            [=](uint16 addr, uint8 data) { vram.mem[addr & ~0xE0] = data; });
}

void PPU::VRAM::inc_horzpos()
{
    // this version uses an 'if', which usually leads to a branch, which can be
    // costly.
    if ((v & 0x001F) == 31) {
        v &= ~0x001F;
        v ^= 0x0400;
    } else
        v += 1;
    // this versions uses bitwise operations instead.
    // uint8 x = (((v & 0x400) >> 5) | (v & 0x1F)) + 1;
    // v = (v & ~0x41F) | ((x & 0x20) << 5) | (x & 0x1F);
    //
    // although the second version seems better, some benchmarking proves that
    // the first version is better instead. i am still investigating whether
    // there can be a better third version (nesdev mentions this code is
    // "unoptimized")
}

void PPU::VRAM::inc_vertpos()
{
    if ((v & 0x7000) != 0x7000)
        v += 0x1000;
    else {
        v &= ~0x7000;
        int y = (v & 0x03E0) >> 5;
        if (y == 29) {
            y = 0;
            v ^= 0x0800;
        } else if (y == 31)
            y = 0;
        else
            y += 1;
        v = (v & ~0x03E0) | (y << 5);
    }
}

void PPU::VRAM::copy_horzpos()
{
    v = (t & 0x041F) | (v & ~0x41F);
}

void PPU::VRAM::copy_vertpos()
{
    v = (t & 0x7BE0) | (v & ~0x7EB0);
}

void PPU::Background::fetch_nt(bool dofetch)
{
    if (!dofetch)
        return;
    latch.nt = ppu.bus->read(0x2000 | (ppu.vram.v & 0x0FFF));
}

void PPU::Background::fetch_attr(bool dofetch)
{
    // 0x23C0 | NN | YYY | XXX
    if (!dofetch)
        return;
    latch.attr = ppu.bus->read(0x23C0
                              | (ppu.vram.v      & 0x0C00)
                              | (ppu.vram.v >> 4 & 0x0038)
                              | (ppu.vram.v >> 2 & 0x0007));
}

void PPU::Background::fetch_lowbg(bool dofetch)
{
    if (!dofetch)
        return;
    latch.lowbg = ppu.bus->read(0x1000*pt_addr + latch.nt);
}

void PPU::Background::fetch_highbg(bool dofetch)
{
    if (!dofetch)
        return;
    latch.hibg  = ppu.bus->read(0x1000*pt_addr + latch.nt+8);
}

void PPU::Background::shift_run()
{
    shift.bglow   >>= 1;
    shift.bghigh  >>= 1;
    shift.athigh >>= 1;
    shift.athigh |= shift.latchhigh << 7;
    shift.atlow >>= 1;
    shift.atlow |= shift.latchlow  << 7;
}

void PPU::Background::shift_fill()
{
    uint16 v = ppu.vram.v;
    shift.bglow  = latch.lowbg << 8 | (shift.bglow  & 0xFF);
    shift.bghigh = latch.hibg  << 8 | (shift.bghigh & 0xFF);
    // TODO: this doesn't do what you think it does.
    uint8 attr_mask = 0b11 << (~((v >> 1 & 1) | (v >> 6 & 1)))*2;
    shift.latchhigh = latch.attr & attr_mask;
    shift.latchlow  = latch.attr & attr_mask;
}

uint8 PPU::Background::output()
{
    uint8 mask      = 1 << ppu.vram.fine_x;
    bool lowbit     = shift.bglow  & mask;
    bool hibit      = shift.bghigh & mask;
    bool at1        = shift.athigh & mask;
    bool at2        = shift.atlow  & mask;
    uint8 pal       = at1   << 1 | at2;
    uint8 palind    = hibit << 1 | lowbit;
    return ppu.getcolor(0, pal, palind);
}

} // namespace Core


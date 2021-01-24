#include <emu/core/ppu.hpp>

#include <cstdio>
#include <cassert>
#include <cstring> // memset
#include <functional>
#include <fmt/core.h>
#include <emu/core/cpu.hpp>
#include <emu/core/cartridge.hpp>
#define DEBUG
#include <emu/util/debug.hpp>
#include <emu/util/file.hpp>

namespace Core {

#define INSIDE_PPU_CPP
#include "ppumain.cpp"
#include "vram.cpp"
#include "background.cpp"
#include "oam.cpp"
#undef INSIDE_PPU_CPP

void PPU::power()
{
    mapbus();
    vram.power();
    bg.power();
    oam.power();
    // ppu starts at the top of the picture
    lines = 0;
    cycles = 0;
    nmi_enabled   = 0;
    ext_bus_dir   = 0;
    effects.grey  = 0;
    effects.red   = 0;
    effects.green = 0;
    effects.blue  = 0;
    vblank  = 0;
    spr0hit = 0;
    spr_ov  = 1;
    odd_frame = 0;
    // randomize oam, palette, nt ram, chr ram
}

void PPU::reset()
{
    vram.reset();
    bg.reset();
    oam.reset();
    nmi_enabled   = 0;
    ext_bus_dir   = 0;
    effects.grey  = 0;
    effects.red   = 0;
    effects.green = 0;
    effects.blue  = 0;
    // spr0hit = random
    // sprov = random
    odd_frame = 0;
    // randomize oam
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
        io_latch = oam.data;
        return io_latch;

    case 0x2007:
        if (vram.v <= 0x3EFF) {
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
        nmi_enabled         = data & 0x80;
        ext_bus_dir         = data & 0x40;
        vram.inc            = (data & 0x04) ? 32 : 1;
        oam.sprsize         = data & 0x20;
        bg.patterntab_addr  = data & 0x10;
        oam.patterntab_addr = data & 0x08;
        bg.nt_base_addr     = data & 0x03;
        vram.t        = (data & 0x03) | (vram.t & 0xF3FF);
        break;

    case 0x2001:
        effects.grey   = data & 0x01;
        bg.show_leftmost    = data & 0x02;
        oam.show_leftmost   = data & 0x04;
        bg.show             = data & 0x08;
        oam.show            = data & 0x10;
        effects.red         = data & 0x20;
        effects.green       = data & 0x40;
        effects.blue        = data & 0x80;
        break;

    case 0x2002:
        break;

    case 0x2003:
        oam.addr = data;
        break;

    case 0x2004:
        oam.data = data;
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

void PPU::mapbus()
{
    std::function<uint16(uint16)> address_nametab;
    std::function<uint8(uint16)> reader;
    std::function<void(uint16, uint8)> writer;

    switch (mirroring) {
    case 0: address_nametab = [](uint16 x) { return x &= ~0x800; }; break;
    case 1: address_nametab = [](uint16 x) { return x &= ~0x400; }; break;
    default: assert(false);
    }

    // reader = [=](uint16 addr) { return cart->read_chrrom(addr); };
    // writer = [=](uint16 addr, uint8 data) { };
    // bus.map(0, 0x2000, reader, writer);

    reader = [=](uint16 addr)             { return readreg(addr); };
    writer = [=](uint16 addr, uint8 data) { writereg(addr, data); };
    cpubus->map(0x2000, 0x2008, reader, writer);

    reader = [=](uint16 addr)             { return vram.mem[address_nametab(addr)]; };
    writer = [=](uint16 addr, uint8 data) { vram.mem[address_nametab(addr)] = data; };
    bus->map(0x2000, 0x3F00, reader, writer);

    reader = [=](uint16 addr)             { return vram.mem[addr & ~0xE0]; };
    writer = [=](uint16 addr, uint8 data) { vram.mem[addr & ~0xE0] = data; };
    bus->map(0x3F00, 0x4000, reader, writer);
}

std::string PPU::getinfo()
{
    return fmt::format("line = {}; cycle = {}; v = {}", lines%262, cycles%341, vram.v);
}

} // namespace Core


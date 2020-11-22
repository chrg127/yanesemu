#include <emu/core/ppu.hpp>

#include <cstdio>
#include <cassert>
#include <cstring> // memset
#include <functional>
#define DEBUG
#include <emu/utils/debug.hpp>

#define INSIDE_PPU_CPP

namespace Core {

void PPU::power(const ROM &chrrom, int mirroring)
{
    vram.power(chrrom, mirroring);
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
    // small note to myself:
    // most games check this in an infinite loop
    // to break out of the loop, set this to 1
    vblank  = 0;
    spr0hit = 0;
    spr_ov  = 1;
    latch.toggle = 0;
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
    latch.toggle = 0;
    odd_frame = 0;
    // randomize oam
}

uint8_t PPU::readreg(const uint16_t which)
{
    switch (which) {
    case 0x2000: case 0x2001: case 0x2003:
    case 0x2005: case 0x2006: case 0x4014:
        break;

    case 0x2002:
        io_latch |= (vblank << 7 | spr0hit << 6 | spr_ov << 5);
        vblank = 0;
        latch.toggle = 0;
        return io_latch;

    case 0x2004:
        io_latch = oam.data;
        return io_latch;

    case 0x2007:
        io_latch = vram.readdata();
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

void PPU::writereg(const uint16_t which, const uint8_t data)
{
    io_latch = data;
    switch (which) {
    case 0x2000:
        nmi_enabled         = data & 0x80;
        ext_bus_dir         = data & 0x40;
        vram.increment      = (data & 0x04) ? 32 : 1;
        oam.sprsize         = data & 0x20;
        bg.patterntab_addr  = data & 0x10;
        oam.patterntab_addr = data & 0x08;
        bg.nt_base_addr     = data & 0x03;
        vram.tmp.reg        = (data & 0x03) | (vram.tmp.reg & 0xF3FF);
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
        if (latch.toggle == 0) {
            vram.tmp = (data &  0xF8) >> 3  | (vram.tmp & ~(0xF8 >> 3));
            vram.fine_x = data & 0x7;
        } else {
            vram.tmp = (data &  0x07) << 12 | (vram.tmp & ~(0x07 << 12));
            vram.tmp = (data & ~0xF8) << 2  | (vram.tmp & ~(0xF8 << 2 ));
        }
        latch.toggle ^= 1;
        break;

    case 0x2006:
        if (latch.toggle == 0) {
            vram.tmp.high = data & 0x3F;
        } else {
            vram.tmp.low = data;
            vram.vaddr = vram.tmp.reg;
        }
        latch.toggle ^= 1;
        break;

    case 0x2007:
        vram.writedata(data);
        break;

#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
}

void PPU::output_pixel()
{
    uint8_t bgpixel = output_bgpixel();
    // uint24 sppixel = output_sppixel();
    assert(lines <= 256 && cycles <= 239);
    output[lines*239+cycles] = bgpixel;
}

#include "ppumain.cpp"
#include "vram.cpp"
#include "background.cpp"
#include "oam.cpp"
#include "palette.cpp"

} // namespace Core

#undef INSIDE_PPU_CPP


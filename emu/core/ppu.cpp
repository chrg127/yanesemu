#include <emu/core/ppu.hpp>

#include <cassert>
#include <cstring> // memset
#define DEBUG
#include <emu/utils/debug.hpp>

#define INSIDE_PPU_CPP

namespace Core {

void PPU::power(const ROM &chrrom, int mirroring)
{
    vram.power(chrrom, mirroring);
    bg.power();
    oam.power();
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
    sprov   = 1;
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

void PPU::main()
{
    int x = get_x(cycle), y = get_y(cycle);
    if (y >= 0 && y <= 239 &&
       (x >= 1 && x <= 256) || (x >= 321 && x <= 340)) {
        bg_cycle(x, y);
    }

    if (x == 241 && y == 1) {
        vblank = 1;
        return;
    }
}

uint8_t PPU::readreg(const uint16_t which)
{
    switch (which) {
    case 0x2000: case 0x2001: case 0x2003:
    case 0x2005: case 0x2006: case 0x4014:
        break;

    case 0x2002:
        io_latch |= (vblank << 7 | spr0hit << 6 | sprov << 5);
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
            vram.tmp.reg = (data & 0xF8) | (vram.tmp.reg & 0xFFE0);
            vram.finex   = (data & 0x7);
        } else {
            vram.tmp.reg = (data & 0x07) | (vram.tmp.reg & 0x8FFF);
            vram.tmp.reg = (data & 0xF8) | (vram.tmp.reg & 0xFC1F);
        }
        latch.toggle ^= 1;
        break;

    case 0x2006:
        if (latch.toggle == 0) {
            vram.tmp.high = data & 0x3F;
        } else {
            vram.tmp.low = data;
            vram.addr = vram.tmp.reg;
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

// void PPU::scanline_render()
// {
//     int col = getcol();
//     if ((col >= 1 && col <= 256) || (col >= 321 && col <= 340)) {
//         switch (col % 8) {
//         case 1: case 2: bg.fetch_nt(); break;
//         case 3: case 4: bg.fetch_at(); break;
//         case 5: case 6: bg.fetch_lowbg(); break;
//         case 7: case 0:
//             bg.fetch_highbg();
//             // inc_v();
//             break;
//         }
//     }

//     if (getrow() == 261 && col == 1) {
//         // clear_stuff();
//     }
// }

void PPU::printinfo(Utils::File &log)
{
    log.printf("lineno: %d, linec: %d\n", getrow(), getcol());
}

void PPU::dot256()
{
    int tmp = vram.increment;
    vram.increment = 32;
    vram.incv();
    vram.increment = tmp;
}

#include <emu/core/vram.cpp>
#include <emu/core/background.cpp>
#include <emu/core/oam.cpp>

} // namespace Core

#undef INSIDE_PPU_CPP


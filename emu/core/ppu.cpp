#include <emu/core/ppu.hpp>

#include <cassert>
#include <cstring> // memset
#define DEBUG
#include <emu/utils/debug.hpp>

namespace Core {

void PPU::power(uint8_t *chrrom)
{
    bus.initmem(chrrom);
    bg.power();
    oam.power();
}

void PPU::reset()
{
}

void PPU::main()
{
    if (vblank) {
        scanline_render();
        return;
    }
    if (getrow() == 241 && getcol() == 1) {
        vblank = 1;
        return;
    }
    // else
    scanline_empty();
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
        io_latch = bg.readdata();
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
        bg.vram_increment = (data & 0x04) ? 32 : 1;
        oam.sprsize         = data & 0x20;
        bg.patterntab_addr  = data & 0x10;
        oam.patterntab_addr = data & 0x08;
        bg.nt_base_addr = data & 0x03;
        bg.tmp = (data & 0x03) | (bg.tmp & 0xF3FF);
        break;

    case 0x2001:
        effects.greyscale        = data & 0x01;
        bg.show_leftmost = data & 0x02;
        oam.show_leftmost        = data & 0x04;
        bg.show          = data & 0x08;
        oam.show                 = data & 0x10;
        effects.red              = data & 0x20;
        effects.green            = data & 0x40;
        effects.blue             = data & 0x80;
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
            bg.tmp = (data & 0xF8) | (bg.tmp & 0xFFE0);
            bg.finex = (data & 0x7);
        } else {
            bg.tmp = (data & 0x07) | (bg.tmp & 0x8FFF);
            bg.tmp = (data & 0xF8) | (bg.tmp & 0xFC1F);
        }
        latch.toggle ^= 1;
        break;

    case 0x2006:
        if (latch.toggle == 0) {
            bg.tmp = (data & 0x3F) | (bg.tmp & 0xC0FF);
        } else {
            bg.tmp = (bg.tmp & 0xFF00) | data;
            bg.vram_addr = bg.tmp;
        }
        latch.toggle ^= 1;
        break;

    case 0x2007:
        bg.writedata(data);
        break;

    case CPUMap::PPU_OAM_DMA:
        oam.dma = data;
        break;

#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
}

void PPU::scanline_render()
{
    int col = getcol();
    if ((col >= 1 && col <= 256) || (col >= 321 && col <= 340)) {
        switch (col % 8) {
        case 1: case 2: bg.fetch_nt(); break;
        case 3: case 4: bg.fetch_at(); break;
        case 5: case 6: bg.fetch_lowbg(); break;
        case 7: case 0:
            bg.fetch_highbg();
            // inc_v();
            break;
        }
    }

    if (getrow() == 261 && col == 1) {
        // clear_stuff();
    }
}

void PPU::scanline_empty()
{

}

void PPU::printinfo(IO::File &log)
{
    log.printf("lineno: %d, linec: %d\n", getrow(), getcol());
}

} // namespace Core


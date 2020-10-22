#include <emu/core/ppu.hpp>

#include <cassert>
#include <cstring> // memset
#include <emu/core/ppubus.hpp>
#define DEBUG
#include <emu/utils/debug.hpp>

namespace Core {

void PPU::power(uint8_t *chrrom)
{
    bus->initmem(chrrom);
    ctrl = mask = oam_addr = 0;
    addr_latch.clear();
    status = 0b10100000;
    // oam, palette, nt ram and chr ram are unspecified. let's clear them all to 0 for now.
    std::memset(oam, 0, PPUMap::OAM_SIZE);
}

void PPU::reset()
{
    ctrl = mask = 0;
    status &= 0b01111111;
    addr_latch.latch = 0;
    addr_latch.scroll = 0;
    // we can leave other memory areas alone
}

void PPU::main()
{
    if (status & PPU_STATUS_VBLANK) {
        scanline_render();
        return;
    }
    if (lineno == 241 && linec == 1) {
        status |= PPU_STATUS_VBLANK;
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
        io_latch |= (status & 0xE0);
        status &= 0x60; // set vblank / nmi_occurred to false
        addr_latch.latch = 0;
        addr_latch.toggle = 0;
        break;

    case 0x2004:
        io_latch = oam_data;
        break;
    case 0x2007:
        io_latch = ppu_data;
        vram.addr += ((ctrl & PPU_CTRL_VRAM_INCREMENT) == 0) ? 1 : 32;
        break;

#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
    return io_latch;
}

void PPU::writereg(const uint16_t which, const uint8_t data)
{
    io_latch = data;
    switch (which) {
    case 0x2000:
        nmi_enabled = data & 0x80;
        ext_bus_dir = data & 0x40;
        vram_increment = (data & 0x04) ? 32 : 1;
        oam.sprsize = data & 0x20;
        background.patterntab_addr = data & 0x10;
        oam.patterntab_addr = data & 0x08;
        background.nt_base_addr = data & 0x03;
        background.tmp = (data & 0x03) | (background.tmp & 0xF3FF);
        break;

    case 0x2001:
        effects.greyscale        = data & 0x01;
        background.show_leftmost = data & 0x02;
        oam.show_leftmost        = data & 0x04;
        background.show          = data & 0x08;
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
        addr_latch.scroll = data << 8 | addr_latch.latch;
        addr_latch.latch = data;
        if (addr_latch.toggle == 0) {
            background.tmp = (data & 0xF8) | (background.tmp & 0xFFE0);
            vram.fine_x_scroll = (data & 0x7);
        } else {
            background.tmp = (data & 0x07) | (background.tmp & 0x8FFF);
            background.tmp = (data & 0xC0) | (background.tmp & 0xF3FF);
            background.tmp = (data & 0xC0) | (background.tmp & 0xFF1F);
        }
        addr_latch.toggle ^= 1;
        break;

    case 0x2006:
        addr_latch.addr = data << 8 | addr_latch.latch;
        addr_latch.latch = data;
        if (addr_latch.toggle == 0)
            background.tmp = (data & 0x3F) | (background.tmp & 0x80);
        else {
            background.tmp = (background.tmp & 0xFF00) | data;
            background.addr = background.tmp;
        }
        addr_latch.toggle ^= 1;
        break;

    case 0x2007:
        ppu_data = data;
        background.addr += vram_increment;
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
        switch (linec % 8) {
        case 1: case 2: fetch_nt(); break;
        case 3: case 4: fetch_at(); break;
        case 5: case 6: fetch_lowbg(); break;
        case 7: case 0:
            fetch_highbg();
            // inc_v();
            break;
        }
    }

    if (lineno == 261 && linec == 1) {
        // clear_stuff();
    }
}

void PPU::scanline_empty()
{

}

void PPU::fetch_nt()
{
    internal_latch.nt = (*bus)[(uint16_t) (PPUMap::NAME_TAB0_START | (vram.addr & 0x0FFF))];
}

void PPU::fetch_at()
{
    internal_latch.at = (*bus)[0x23C0 | (vram.addr & 0x0C00) |
               ((vram.addr >> 4) & 0x38) | ((vram.addr >> 2) & 0x07)];
}

void PPU::fetch_lowbg()
{
    internal_latch.lowbg = (*bus)[internal_latch.nt];
}

void PPU::fetch_highbg()
{
    internal_latch.hibg = (*bus)[internal_latch.nt+8];
}

void PPU::printinfo(IO::File &log)
{
    log.printf("lineno: %d, linec: %d\n", lineno, linec);
}

} // namespace Core


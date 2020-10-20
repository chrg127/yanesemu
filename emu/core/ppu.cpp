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
    case CPUMap::PPU_CTRL:      break;
    case CPUMap::PPU_MASK:      break;
    case CPUMap::PPU_STATUS:
        io_latch |= (status & 0xE0);
        status &= 0x60; // set vblank / nmi_occurred to false
        addr_latch.latch = 0;
        addr_latch.toggle = 0;
        break;
    case CPUMap::PPU_OAM_ADDR:  break;
    case CPUMap::PPU_OAM_DATA:
        io_latch = oam_data;
        break;
    case CPUMap::PPU_SCROLL:    break;
    case CPUMap::PPU_ADDR:      break;
    case CPUMap::PPU_DATA:
        io_latch = ppu_data;
        vram.addr += ((ctrl & PPU_CTRL_VRAM_INCREMENT) == 0) ? 1 : 32;
        break;
    case CPUMap::PPU_OAM_DMA:   break;
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
    case CPUMap::PPU_CTRL:
        vram.tmp = (io_latch & 0x3) | (vram.tmp & 0xF3FF);
        ctrl = io_latch;
        break;
    case CPUMap::PPU_MASK:
        mask = io_latch;
        break;
    case CPUMap::PPU_STATUS:
        break;
    case CPUMap::PPU_OAM_ADDR:
        oam_addr = io_latch;
        break;
    case CPUMap::PPU_OAM_DATA:
        oam_data = io_latch;
        break;

    case CPUMap::PPU_SCROLL:
        addr_latch.scroll = io_latch << 8 | addr_latch.latch;
        addr_latch.latch = io_latch;
        if (addr_latch.toggle == 0) {
            vram.tmp = (io_latch & 0xF8) | (vram.tmp & 0xFFE0);
            vram.fine_x_scroll = (io_latch & 0x7);
        } else {
            vram.tmp = (io_latch & 0x7) | (vram.tmp & 0x8FFF);
            vram.tmp = (io_latch & 0xC0) | (vram.tmp & 0xF3FF);
            vram.tmp = (io_latch & 0xC0) | (vram.tmp & 0xFF1F);
        }
        addr_latch.toggle ^= 1;
        break;

    case CPUMap::PPU_ADDR:
        addr_latch.addr = io_latch << 8 | addr_latch.latch;
        addr_latch.latch = io_latch;
        if (addr_latch.toggle == 0)
            vram.tmp = (io_latch & 0x3F) | (vram.tmp & 0x80);
        else {
            vram.tmp = (vram.tmp & 0xFF00) | io_latch;
            vram.addr = vram.tmp;
        }
        addr_latch.toggle ^= 1;
        break;

    case CPUMap::PPU_DATA:
        ppu_data = io_latch;
        vram.addr += ((ctrl & PPU_CTRL_VRAM_INCREMENT) == 0) ? 1 : 32;
        break;
    case CPUMap::PPU_OAM_DMA:
        oam_dma = io_latch;
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
    // bg event first
    if ((linec >= 1 && linec <= 256) ||
        (linec >= 321 && linec <= 340)) {
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


#include <emu/core/ppu.hpp>

#include <cassert>
#include <cstring>
#include <emu/core/ppubus.hpp>

#define DEBUG

namespace Core {

void PPU::power(uint8_t *chrrom)
{
    bus->initmem(chrrom);
    ctrl = mask = oam_addr = 0;
    scroll.clear();
    address.clear();
    status = 0b10100000;
    // oam, palette, nt ram and chr ram are unspecified. let's clear them all to
    // 0 for now.
    std::memset(oam, 0, PPUMap::OAM_SIZE);
}

void PPU::reset()
{
    ctrl = mask = address.latch = 0;
    // only scroll reg is fully cleared at reset
    scroll.clear();
    status &= 0b01111111;
    // we can leave other memory areas alone
    did_reset = true;
}

/* PPU I/O consists of an internal latch that is filled with data when writing
 * and is filled and returned for reading */
uint8_t PPU::readreg(const uint16_t which)
{
    switch (which) {
    case CPUMap::PPU_CTRL:     break;
    case CPUMap::PPU_MASK:     break;
    case CPUMap::PPU_STATUS:
        // ppu_io_latch |= (status & 0xE0);
        // scroll.latch = address.latch = false
        break;
    case CPUMap::PPU_OAM_ADDR: break;
    case CPUMap::PPU_OAM_DATA: ppu_io_latch = oam_data; break;
    case CPUMap::PPU_SCROLL:   break;
    case CPUMap::PPU_ADDR:     break;
    case CPUMap::PPU_DATA:     ppu_io_latch = ppu_data; break;
    case CPUMap::PPU_OAM_DMA:  break;
#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
    return ppu_io_latch;
}

void PPU::writereg(const uint16_t which, const uint8_t data)
{
    ppu_io_latch = data;
    switch (which) {
    case CPUMap::PPU_CTRL:     ctrl = ppu_io_latch; break;
    case CPUMap::PPU_MASK:     mask = ppu_io_latch; break;
    case CPUMap::PPU_STATUS:   break;
    case CPUMap::PPU_OAM_ADDR: oam_addr = ppu_io_latch; break;
    case CPUMap::PPU_OAM_DATA: oam_data = ppu_io_latch; break;
    case CPUMap::PPU_SCROLL:   break;   // write twice
    case CPUMap::PPU_ADDR:     break;   // write twice
    case CPUMap::PPU_DATA:     ppu_data = ppu_io_latch; break;
    case CPUMap::PPU_OAM_DMA:  oam_dma = ppu_io_latch; break;
#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
}

} // namespace Core


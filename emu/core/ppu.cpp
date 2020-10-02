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
    did_reset = true;
}

/* PPU I/O consists of an internal latch that is filled with data when writing
 * and is filled and returned for reading */
uint8_t PPU::readreg(const uint16_t which)
{
    switch (which) {
    case CPUMap::PPU_CTRL:      break;
    case CPUMap::PPU_MASK:      break;
    case CPUMap::PPU_STATUS:
        io_latch |= (status & 0xE0);
        addr_latch.clear();
        break;
    case CPUMap::PPU_OAM_ADDR:  break;
    case CPUMap::PPU_OAM_DATA:  io_latch = oam_data; break;
    case CPUMap::PPU_SCROLL:    break;
    case CPUMap::PPU_ADDR:      break;
    case CPUMap::PPU_DATA:      io_latch = ppu_data; break;
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
    case CPUMap::PPU_CTRL:     ctrl = io_latch; break;
    case CPUMap::PPU_MASK:     mask = io_latch; break;
    case CPUMap::PPU_STATUS:   break;
    case CPUMap::PPU_OAM_ADDR: oam_addr = io_latch; break;
    case CPUMap::PPU_OAM_DATA: oam_data = io_latch; break;
    case CPUMap::PPU_SCROLL:
        addr_latch.scroll = data << 8 | addr_latch.latch;
        addr_latch.latch = data;
        break;
    case CPUMap::PPU_ADDR:
        addr_latch.addr = data << 8 | addr_latch.latch;
        addr_latch.latch = data;
        break;
    case CPUMap::PPU_DATA:     ppu_data = io_latch; break;
    case CPUMap::PPU_OAM_DMA:  oam_dma = io_latch; break;
#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
}

} // namespace Core


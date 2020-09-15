#include <emu/core/ppu.hpp>

#include <cassert>

#define DEBUG

namespace Core {

uint8_t PPU::readreg(const uint16_t which) const
{
    switch (which) {
    case 0x2002: return status;
    case 0x2004: return oam_data;
    case 0x2007: return ppu_data;
#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
}

void PPU::writereg(const uint16_t which, const uint8_t data)
{
    switch (which) {
    case 0x2000: ctrl = data; break;
    case 0x2001: mask = data; break;
    case 0x2003: oam_addr = data; break;
    case 0x2004: oam_data = data; break;
    case 0x2007: ppu_data = data; break;
    case 0x4014: oam_dma = data; break;
#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
}

} // namespace Core


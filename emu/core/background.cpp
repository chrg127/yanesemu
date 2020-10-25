#include <emu/core/background.hpp>

void PPU::Background::power()
{

}

void PPU::Background::fetch_nt()
{
    // internal_latch.nt = vram[vram.addr];
}

void PPU::Background::fetch_at()
{
    // internal_latch.at = vram[0x23C0 | (vram.addr & 0x0C00) |
    //            ((vram.addr >> 4) & 0x38) | ((vram.addr >> 2) & 0x07)];
}

void PPU::Background::fetch_lowbg()
{
    // internal_latch.lowbg = vram[internal_latch.nt];
}

void PPU::Background::fetch_highbg()
{
    // internal_latch.hibg = vram[internal_latch.nt+8];
}


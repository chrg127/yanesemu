#include <emu/core/background.hpp>

void Background::power()
{

}

uint8_t Background::read(uint16_t addr)
{

}

void Background::write(uint16_t addr, uint8_t data)
{

}

void Background::fetch_nt()
{
    internal_latch.nt = (*bus)[(uint16_t) (0x2000 | (vram.addr & 0x0FFF))];
}

void Background::fetch_at()
{
    internal_latch.at = (*bus)[0x23C0 | (vram.addr & 0x0C00) |
               ((vram.addr >> 4) & 0x38) | ((vram.addr >> 2) & 0x07)];
}

void Background::fetch_lowbg()
{
    internal_latch.lowbg = (*bus)[internal_latch.nt];
}

void Background::fetch_highbg()
{
    internal_latch.hibg = (*bus)[internal_latch.nt+8];
}

uint8_t Background::readdata()
{

}

void Background::writedata(uint8_t data)
{

}


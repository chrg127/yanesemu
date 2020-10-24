#include <emu/core/ppubus.hpp>

#include <cstring>
#include <emu/io/file.hpp>

namespace Core {

PPUBus::PPUBus(int mirroring)
{
    if (mirroring == 0) // v-mirror
        get_nt_addr = [](uint16_t x) { return x |= 0x0800; };
    else if (mirroring == 1)
        get_nt_addr = [](uint16_t x) { return x |= 0x0400; };
    // else, mapper defined
}

void PPUBus::initmem(uint8_t *chrrom)
{
    std::memset(memory, 0, 0x4000);
    // the CHR ROM is mapped to both pattern tables
    std::memcpy(memory, chrrom, 0x2000);
}

uint8_t &PPUBus::operator[](const uint16_t addr)
{
    if (addr >= 0x2000 && addr <= 0x3FFF)       // inside nametable space
        return memory[get_nt_addr(0x2000 + (addr & 0x0FFF))];
    else if (addr >= 0x3F00 && addr <= 0x3FFF)  // $3F00 < addr < $3FFF, or inside palette ram space
        return memory[0x3F00 + (addr & 0x00FF) % 0x20];
    else    // everything else
        return memory[addr];
}

} // namespace Core

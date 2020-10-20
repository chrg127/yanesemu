#include <emu/core/ppubus.hpp>

#include <cstring>
#include <emu/io/file.hpp>

namespace Core {

void PPUBus::initmem(uint8_t *chrrom)
{
    std::memset(memory, 0, PPUMap::MEMSIZE);
    // the CHR ROM is mapped to both pattern tables
    std::memcpy(memory, chrrom, PPUMap::PATTERN_TABRIGHT_END+1);
}

// literally the same as Bus::memdump...
void PPUBus::memdump(IO::File &df)
{
    int i, j;

    df.printf("=== PPU memory ===\n");
    if (!df.isopen())
        return;
    for (i = 0; i < CPUMap::MEMSIZE; ) {
        df.printf("%04X: ", i);
        for (j = 0; j < 16; j++) {
            df.printf("%02X ", memory[i]);
            i++;
        }
        df.putc('\n');
    }
    df.putc('\n');
}

uint8_t &PPUBus::operator[](const uint16_t addr)
{
    if (addr >= PPUMap::NAME_TAB0_START && addr <= PPUMap::NAME_TAB_MIRROR_END)
        // $2000 < addr < $3EFF, or inside nametable space
        return memory[0x2000 + (addr & 0x0FFF)];
    else if (addr >= PPUMap::PALRAM_START && addr <= PPUMap::PALRAM_MIRROR_END)
        // $3F00 < addr < $3FFF, or inside palette ram space
        return memory[0x3F00 + (addr & 0x00FF) % 0x20];
    else
        // everything else; ignore array bounds
        return memory[addr];
}

}


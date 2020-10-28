#ifndef INSIDE_PPU_CPP
#error "This file must only be #include'd by ppu.cpp."
#else

PPU::VRAM::VRAM(int mirroring)
{
    std::memset(memory, 0, 0x4000);
    if (mirroring == 0) // v-mirror
        get_nt_addr = [](uint16_t x) { return x |= 0x0800; };
    else if (mirroring == 1)
        get_nt_addr = [](uint16_t x) { return x |= 0x0400; };
    // else, mapper defined
}

void PPU::VRAM::initmem(const ROM &chrrom)
{
    chrrom.copy_to(memory, 0, 0x2000);
}

uint8_t &PPU::VRAM::getref(const uint16_t addr)
{
    if (addr >= 0x2000 && addr <= 0x3FFF)       // inside nametable space
        return memory[get_nt_addr(0x2000 + (addr & 0x0FFF))];
    else if (addr >= 0x3F00 && addr <= 0x3FFF)  // $3F00 < addr < $3FFF, or inside palette ram space
        return memory[0x3F00 + (addr & 0x00FF) % 0x20];
    else
        return memory[addr];
}

/* read from the current vram address and increase the address */
uint8_t PPU::VRAM::read()
{
    uint8_t toret = getref(addr);
    addr += increment;
    return toret;
}

/* write to the current vram address and increase the address */
void PPU::VRAM::write(uint8_t data)
{
    uint8_t &towrite = getref(addr);
    towrite = data;
    addr += increment;
}

#endif

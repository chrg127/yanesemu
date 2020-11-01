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

void PPU::VRAM::power(const ROM &chrrom, int mirroring)
{
    chrrom.copy_to(memory, 0, 0x2000);
    increment = 1; // ctrl = 0
    readbuf     = 0;
    finex   = 0;
    tmp     = 0;
}

void PPU::VRAM::reset()
{
    increment = 1; // ctrl = 0
    readbuf     = 0;
    finex   = 0;
    // tmp = unchanged
}

uint16_t address(uint16_t addr)
{
    if (addr >= 0x2000 && addr <= 0x3FFF)
        return get_nt_addr(0x2000 + (addr & 0x0FFF));
    else if (addr >= 0x3F00 && addr <= 0x3FFF)
        return 0x3F00 + (addr & 0x00FF) % 0x20;
    else
        return addr;
}

uint8_t PPU::VRAM::read()
{
    return memory[address(vaddr)];
}

uint8_t PPU::VRAM::read(uint16_t addr)
{
    return memory[address(addr)];
}

/* write to the current vram address and increase the address */
void PPU::VRAM::write(uint8_t data)
{
    memory[address(vaddr)] = data;
}

void PPU::VRAM::write(uint16_t addr, uint8_t data)
{
    memory[address(addr)] = data;
}

void PPU::VRAM::incv()
{
    vaddr = (vaddr + increment) & 0x7FFF;
}

uint8_t PPU::VRAM::readdata()
{
    uint8_t toret;
    if (vaddr <= 0x3EFF) {
        toret = readbuf;
        readbuf = memory[address(vaddr)];
    } else
        toret = memory[address(vaddr)];
    incv();
    return toret;
}

void PPU::VRAM::writedata(uint8_t data)
{
    memory[address(vaddr++)] = data;
}

#endif

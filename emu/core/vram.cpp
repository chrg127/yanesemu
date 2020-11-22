#ifndef INSIDE_PPU_CPP
#error "This file must only be #include'd by ppu.cpp."
#else

void PPU::VRAM::power(const ROM &chrrom, int mirroring)
{
    std::memset(memory, 0, 0x4000);
    if (mirroring == 0) // v-mirror
        ntaddr = [](uint16_t x) { return x |= 0x0800; };
    else if (mirroring == 1)
        ntaddr = [](uint16_t x) { return x |= 0x0400; };
    // else, mapper defined
    chrrom.copy_to(memory, 0, 0x2000);
    increment = 1; // ctrl = 0
    readbuf     = 0;
    fine_x   = 0;
    tmp     = 0;
}

void PPU::VRAM::reset()
{
    increment = 1; // ctrl = 0
    readbuf     = 0;
    fine_x   = 0;
    // tmp = unchanged
}

void PPU::VRAM::inc_horzpos()
{
    // if ((vaddr & 0x001F) == 31) {
    //     v &= ~0x001F;
    //     v ^= 0x0400;
    // } else
    //     v += 1;
    /*
     * >>> def getwhole(x):
     * ...     return ((x & 0x400) >> 5) | (x & 0x1F)
     * >>> def recompose(x): return (x & 0x20) << 5 | (x & 0x1F)
     * >>> def incv6(x):
     * ...     tmp = getwhole(x)+1
     * ...     return (x & ~0x41F) | recompose(tmp)
     */
    uint8_t x = ((vaddr & 0x400) >> 5) | ((vaddr & 0x1F) + 1);
    vaddr = (vaddr & ~0x41F) | ((x & 0x20) << 5) | (x & 0x1F);
}

void PPU::VRAM::inc_vertpos()
{
    if ((vaddr & 0x7000) != 0x7000)
        vaddr += 0x1000;
    else {
        vaddr &= ~0x7000;
        int y = (vaddr & 0x03E0) >> 5;
        if (y == 29) {
            y = 0;
            vaddr ^= 0x0800;
        } else if (y == 31)
            y = 0;
        else
            y += 1;
        vaddr = (vaddr & ~0x03E0) | (y << 5);
    }
}

void PPU::VRAM::copy_horzpos()
{
    // if rendering is enabled
    vaddr = (tmp.reg & 0x41F) | (vaddr & ~0x41F);
}

void PPU::VRAM::copy_vertpos()
{
    vaddr = (tmp.reg & 0x7BE0) | (vaddr & ~0x7EB0);
}

/* gets the actual address given any address. this accounts for mirroring. */
uint16_t PPU::VRAM::address(uint16_t addr)
{
    if (addr >= 0x2000 && addr <= 0x3FFF)
        return ntaddr(0x2000 + (addr & 0x0FFF));
    else if (addr >= 0x3F00 && addr <= 0x3FFF)
        return 0x3F00 + (addr & 0x00FF) % 0x20;
    else
        return addr;
}

uint8_t PPU::VRAM::read(uint16_t addr)
{
    return memory[address(addr)];
}

uint8_t PPU::VRAM::read()
{
    return memory[address(vaddr)];
}

void PPU::VRAM::write(uint8_t data)
{
    memory[address(vaddr)] = data;
}

void PPU::VRAM::write(uint16_t addr, uint8_t data)
{
    memory[address(addr)] = data;
}

uint8_t PPU::VRAM::readdata()
{
    uint8_t toret;
    if (vaddr <= 0x3EFF) {
        toret = readbuf;
        readbuf = memory[address(vaddr)];
    } else
        toret = memory[address(vaddr)];
    inc_horzpos();
    return toret;
}

void PPU::VRAM::writedata(uint8_t data)
{
    memory[address(vaddr++)] = data;
}

#endif

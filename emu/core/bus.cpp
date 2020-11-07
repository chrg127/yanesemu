#ifndef INSIDE_CPU_CPP
#error "this file must be #include'd from cpu.cpp"
#else

void CPU::Bus::init(const ROM &prgrom)
{
    std::memset(memory, 0, CPUMap::MEMSIZE);
    prgrom.copy_to(memory+CPUMap::PRGROM_START, prgrom.getsize() - (CPUMap::PRGROM_SIZE+1), CPUMap::PRGROM_SIZE);
}

uint8_t CPU::Bus::read(uint16_t addr)
{
    // if (addr >= 0x2000 && addr <= 0x2007)
    //     ppu->readreg(addr);
    if (addr == 0x2002)
        return 0xFF;
    return memory[addr];
}

void CPU::Bus::write(uint16_t addr, uint8_t val)
{
    if (!write_enable)
        return;
    // if (addr >= 0x2000 && addr <= 0x2007)
    //     ppu->writereg(addr, val);
    memory[addr] = val;
}

#endif

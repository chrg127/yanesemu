#ifndef INSIDE_CPU_CPP
#error "this file must be #include'd from cpu.cpp"
#else

void CPU::Bus::init(ROM &prgrom)
{
    std::memset(memory, 0, CPUMap::MEMSIZE);
    prgrom.copy_to(memory+CPUMap::PRGROM_START, prgrom.getsize() - (CPUMap::PRGROM_SIZE+1), CPUMap::PRGROM_SIZE);
}

uint8_t CPU::Bus::read(uint16_t addr)
{
    return memory[addr];
}

void CPU::Bus::write(uint16_t addr, uint8_t val)
{
    if (!write_enable)
        return;
    memory[addr] = val;
}

/* prints the contents of current memory to a file with name fname */
// void Bus::memdump(IO::File &df)
// {
//     int i, j;
    
//     df.printf("=== CPU Memory ===\n");
//     if (!df.isopen())
//         return;
//     for (i = 0; i < CPUMap::MEMSIZE; ) {
//         df.printf("%04X: ", i);
//         for (j = 0; j < 16; j++) {
//             df.printf("%02X ", memory[i]);
//             i++;
//         }
//         df.putc('\n');
//     }
//     df.putc('\n');
// }

#endif

#ifndef INSIDE_CPU_CPP
#error "this file must be #include'd from cpu.cpp"
#else

/* copies rom memory and initizializes all other memory to 0 */
void CPU::Bus::initmem(uint8_t *prgrom, std::size_t romsize)
{
    std::memset(memory, 0, CPUMap::MEMSIZE);
    std::memcpy(memory+CPUMap::PRGROM_START, prgrom + romsize - (CPUMap::PRGROM_SIZE+1), CPUMap::PRGROM_SIZE);
}

/* reads memory from the specified address */
uint8_t CPU::Bus::read(uint16_t addr)
{
    return memory[addr];
}

/* writes val to the specified address */
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

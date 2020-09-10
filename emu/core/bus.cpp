#include <emu/core/bus.h>

#include <cstdio>
#include <cstring>

namespace Processor {

/* copies rom memory and initizializes all other memory to 0 */
void Bus::initmem(uint8_t *prgrom, size_t romsize)
{
    std::memset(memory, 0, Mem::PRGROM_START-1);
    std::memcpy(memory+Mem::PRGROM_START, prgrom + romsize - (Mem::PRGROM_SIZE+1),
            Mem::PRGROM_SIZE);
}

/* reads memory from the specified address */
uint8_t Bus::read(uint16_t addr)
{
    return memory[addr];
}

/* writes val to the specified address */
void Bus::write(uint16_t addr, uint8_t val)
{
    if (!write_enable)
        return;
    memory[addr] = val;
}

/* prints the contents of current memory to a file with name fname */
void Bus::memdump(FILE *dumpfile)
{
    int i, j;
    
    if (!dumpfile)
        return;
    for (i = 0; i < Mem::MEMSIZE; ) {
        std::fprintf(dumpfile, "%04X: ", i);
        for (j = 0; j < 16; j++) {
            std::fprintf(dumpfile, "%02X ", memory[i]);
            i++;
        }
        std::fputs("\n", dumpfile);
    }
}

} // namespace Processor

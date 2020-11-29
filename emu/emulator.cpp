#include <emu/emulator.hpp>

#include <emu/core/memorymap.hpp>
#include <emu/util/unsigned.hpp>

using Util::File;
using namespace Core;

void Emulator::init(Cartridge &cart)
{
    cpu.power(cart.get_prgrom());
    cpu.attach_ppu(&ppu);
    ppu.power(cart.get_chrrom(), 0);
}

void Emulator::run()
{
    cpu.main();
    int curr_cycle = cpu.get_cycles();
    int cycle_diff = curr_cycle - cycle;
    // run 3 ppu cycles for 1 cpu cycle
    for (int i = 0; i < cycle_diff*3; i++)
        ppu.main();
    cycle = curr_cycle;
}

void Emulator::log(File &logfile)
{
    cpu.printinfo(logfile);
    logfile.printf("Instruction [%02X] ", cpu.peek_opcode());
    logfile.putstr(cpu.disassemble().c_str());
    logfile.putc('\n');
}

void Emulator::dump(File &dumpfile)
{
    auto dumpmem = [](File &df, const uint8 *mem, const std::size_t size) {
        std::size_t i, j;

        for (i = 0; i < size; i++) {
            df.printf("%04lX: ", i);
            for (j = 0; j < 16; j++)
                df.printf("%02X ", mem[i++]);
            df.putc('\n');
        }
        df.putc('\n');
    };
    dumpmem(dumpfile, cpu.getmemory(), CPUMap::MEMSIZE);
    // dumpmem(dumpfile, ppu.getmemory(), PPUMap::MEMSIZE);
}

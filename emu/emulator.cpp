#include <emu/emulator.hpp>

#include <string_view>
#include <emu/core/memorymap.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/file.hpp>

using Util::File;
using namespace Core;

bool Emulator::init(std::string_view rompath)
{
    if (!cartridge.open(rompath)) {
        error("%s: %s\n", rompath.data(), cartridge.geterr().data());
        return false;
    }
    cpu.attach_bus(&cpu_bus);
    ppu.attach_bus(&ppu_bus, &cpu_bus);
    cartridge.attach_bus(&cpu_bus, &ppu_bus);
    return true;
}

void Emulator::power()
{
    cpu.power();
    ppu.power();
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
    logfile.putstr(cpu.getinfo());
    logfile.putstr(ppu.getinfo());
    logfile.printf("Instruction [%02X] ", cpu.peek_opcode());
    logfile.putstr(cpu.disassemble());
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


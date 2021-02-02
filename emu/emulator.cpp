#include <emu/emulator.hpp>

#include <string_view>
#include <emu/util/unsigned.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/file.hpp>

using namespace Core;

void Emulator::insert_rom(const std::string_view rompath)
{
    cartridge.open(rompath);
    // if (!cartridge.open(rompath)) {
    //     error("%s: %s\n", rompath.data(), cartridge.geterr().data());
    //     return false;
    // }
    // return true;
}

void Emulator::power()
{
    cpu.attach_bus(&cpu_bus);
    ppu.attach_bus(&ppu_bus, &cpu_bus);
    cartridge.attach_bus(&cpu_bus, &ppu_bus);
    cpu.power();
    ppu.power();
}

void Emulator::run()
{
    cpu.run();
    int curr_cycle = cpu.get_cycles();
    int cycle_diff = curr_cycle - cycle;
    // run 3 ppu cycles for 1 cpu cycle
    for (int i = 0; i < cycle_diff*3; i++)
        ppu.run();
    cycle = curr_cycle;
}

// re-initialize the emulator with a new rom file
void Emulator::reset(const std::string_view rompath)
{
    cpu.reset();
    ppu.reset();
    insert_rom(rompath);
    // if (!insert_rom(rompath))
    //     return false;
    power();
    // return true;
}

void Emulator::log(Util::File &logfile)
{
    if (!logfile)
        return;
    logfile.putstr(cpu.get_info() + ' ');
    logfile.putstr(ppu.get_info() + ' ');
    logfile.printf("Instruction [%02X] ", cpu.peek_opcode());
    logfile.putstr(cpu.disassemble());
    logfile.putc('\n');
}

template <std::size_t Size>
static void dump_to(Util::File &df, const std::array<uint8, Size> mem) {
    for (std::size_t i = 0; i < Size; i++) {
        df.printf("%04lX: ", i);
        for (std::size_t j = 0; j < 16; j++)
            df.printf("%02X ", mem[i++]);
        df.putc('\n');
    }
    df.putc('\n');
}

void Emulator::dump(Util::File &dumpfile)
{
    dump_to(dumpfile, cpu.get_memory());
    // dump_to(dumpfile, ppu.getmemory(), PPUMap::MEMSIZE);
}


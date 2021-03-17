#include <emu/core/emulator.hpp>

#include <string_view>
#include <fmt/core.h>
#include <emu/util/unsigned.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/file.hpp>

using namespace Core;

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

// void Emulator::log(Util::File &logfile)
// {
//     if (!logfile)
//         return;
//     logfile.print(cpu.get_info() + ' ');
//     logfile.print(ppu.get_info() + ' ');
//     logfile.print(cpu.disassemble());
//     logfile.putc('\n');
// }

/*
void Emulator::dump(Util::File &dumpfile)
{
    if (!dumpfile)
        return;
    auto dump_mem = [&](Bus &bus) {
        for (std::size_t i = 0; i < bus.size(); ) {
            dumpfile.print("{:04X}: ", i);
            for (std::size_t j = 0; j < 16; j++) {
                dumpfile.print("{:02X} ", bus.read(i));
                i++;
            }
            dumpfile.putc('\n');
        }
        dumpfile.putc('\n');
    };
    dump_mem(rambus);
    dump_mem(vrambus);
}
*/

// void Emulator::run_frame(Util::File &logfile)
// {
//     while (!nmi) {
//         log(logfile);
//         run();
//     }
//     nmi = false;
// }

bool Emulator::insert_rom(Util::File &romfile)
{
    if (!cartridge.parse(romfile))
        return false;
    ppu.set_mirroring(cartridge.mirroring());
    cartridge.attach_bus(&rambus, &vrambus);
    return true;
}


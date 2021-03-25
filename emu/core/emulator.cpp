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

void Emulator::run_frame()
{
    if (debugger_has_quit())
        return;
    while (!nmi)
        run();
    nmi = false;
}

bool Emulator::insert_rom(Util::File &romfile)
{
    if (!cartridge.parse(romfile))
        return false;
    ppu.set_mirroring(cartridge.mirroring());
    cartridge.attach_bus(&rambus, &vrambus);
    return true;
}


#include <emu/core/emulator.hpp>

#include <string_view>
#include <fmt/core.h>
#include <emu/core/cartridge.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/file.hpp>

using namespace Core;

void Emulator::run()
{
    unsigned long old_cycle = cpu.cycles();
    cpu.run();
    unsigned long delta = cpu.cycles() - old_cycle;
    // run 3 ppu cycles for 1 cpu cycle
    for (unsigned long i = 0; i < delta*3; i++)
        ppu.run();
}

void Emulator::run_frame()
{
    while (!nmi)
        run();
    nmi = false;
}

bool Emulator::insert_rom(Util::File romfile)
{
    auto opt_data = parse_cartridge(romfile);
    if (!opt_data)
        return false;
    Cartridge::Data &data = opt_data.value();
    fmt::print("{}\n", data.to_string());
    prgrom = std::move(data.prgrom);
    chrrom = std::move(data.chrrom);
    rambus.map(CARTRIDGE_START, 0x8000,
            [](uint16 addr) { return 0; },
            [](uint16 addr, uint8 data) { /***********/ });
    rambus.map(0x8000, CPUBUS_SIZE,
            [this](uint16 addr)
            {
                uint16 offset = addr - 0x8000;
                uint16 start = prgrom.size() - 0x8000;
                uint16 eff_addr = start + offset;
                return prgrom[eff_addr];
            },
            [](uint16 addr, uint8 data) { /***********/ });
    vrambus.map(PT_START, NT_START,
            [this](uint16 addr) { return chrrom[addr]; },
            [](uint16 addr, uint8 data) { /***********/ });
    ppu.set_mirroring(data.mirroring);
    return true;
}


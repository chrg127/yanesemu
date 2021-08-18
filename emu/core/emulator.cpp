#include <emu/core/emulator.hpp>

#include <emu/util/uint.hpp>

using namespace Core;

Emulator::Emulator()
{
    ppu.on_nmi([this](bool nmi_enabled) {
        nmi = true;
        if (nmi_enabled)
            cpu.fire_nmi();
    });
}

void Emulator::power(bool reset)
{
    cpu.power(reset);
    ppu.power(reset);
    memory.power(reset);
}

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

void Emulator::map(Mirroring mirroring)
{
    rambus.reset();
    vrambus.reset();

    memory.map(rambus, vrambus, mirroring);

    rambus.map(APU_START, CARTRIDGE_START,
        [this](uint16 addr)             { return cpu.readreg(addr); },
        [this](uint16 addr, uint8 data) { cpu.writereg(addr, data); });

    rambus.map(PPUREG_START, APU_START,
        [this](uint16 addr)             { return ppu.readreg(0x2000 + (addr & 0x7)); },
        [this](uint16 addr, uint8 data) { ppu.writereg(0x2000 + (addr & 0x7), data); });

    rambus.map(CARTRIDGE_START, 0x8000,
        [](uint16 addr)             { return 0; },
        [](uint16 addr, uint8 data) { /**/ });

    rambus.map(0x8000, CPUBUS_SIZE,
        [this](uint16 addr)
        {
            uint16 offset = addr - 0x8000;
            uint16 start = prgrom.size() - 0x8000;
            uint16 eff_addr = start + offset;
            return prgrom[eff_addr];
        },
        [](uint16 addr, uint8 data) { /**/ });

    vrambus.map(PT_START, NT_START,
        [this](uint16 addr)         { return chrrom[addr]; },
        [](uint16 addr, uint8 data) { /**/ });
}

void Emulator::insert_rom(Cartridge::Data &&cartdata)
{
    prgrom = cartdata.prgrom;
    chrrom = cartdata.chrrom;
    map(cartdata.mirroring);
}


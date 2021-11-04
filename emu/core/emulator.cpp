#include "emulator.hpp"

#include <emu/program.hpp>

namespace core {

Emulator emulator;

Emulator::Emulator()
{
    ppu.on_nmi([this](bool nmi_enabled) {
        nmi = true;
        program.video_frame(screen.data());
        if (nmi_enabled)
            cpu.fire_nmi();
    });
}

void Emulator::power(bool reset)
{
    cpu.power(reset);
    ppu.power(reset);
    memory.power(reset);
    port.load(Controller::Type::Gamepad);
}

void Emulator::run()
{
    if (emu_stop)
        return;
    unsigned long old_cycle = cpu.cycles();
    cpu.run();
    unsigned long delta = cpu.cycles() - old_cycle;
    // run 3 ppu cycles for 1 cpu cycle
    for (unsigned long i = 0; i < delta*3; i++)
        ppu.run();
}

void Emulator::run_frame()
{
    while (!nmi && !emu_stop)
        run();
    nmi = false;
}

void Emulator::map(Mirroring mirroring)
{
    rambus.reset();
    vrambus.reset();

    memory.map(rambus, vrambus, mirroring);

    rambus.map(APU_START, CARTRIDGE_START, [this](u16 addr)          { return cpu.readreg(addr); },
                                           [this](u16 addr, u8 data) { cpu.writereg(addr, data); });
    rambus.map(PPUREG_START, APU_START, [this](u16 addr)          { return ppu.readreg(0x2000 + (addr & 0x7)); },
                                        [this](u16 addr, u8 data) { ppu.writereg(0x2000 + (addr & 0x7), data); });
    rambus.map(CARTRIDGE_START, 0x8000, [this](u16 addr)          { return mapper->read_wram(addr); },
                                        [this](u16 addr, u8 data) { mapper->write_wram(addr, data); });
    rambus.map(0x8000, CPUBUS_SIZE, [this](u16 addr)          { return mapper->read_rom(addr); },
                                    [this](u16 addr, u8 data) { mapper->write_rom(addr, data); });

    vrambus.map(PT_START, NT_START, [this](u16 addr)          { return mapper->read_chr(addr); },
                                    [this](u16 addr, u8 data) { mapper->write_chr(addr, data); });
}


bool Emulator::insert_rom(const Cartridge::Data &cartdata)
{
    prgrom = cartdata.prgrom;
    chrrom = cartdata.chrrom;
    mapper = create_mapper(cartdata.mapper, prgrom, chrrom);
    if (!mapper)
        return false;
    map(cartdata.mirroring);
    return true;
}

} // namespace core

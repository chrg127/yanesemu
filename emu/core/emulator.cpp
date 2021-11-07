#include "emulator.hpp"

#include <emu/program.hpp>

namespace core {

Emulator emulator;

void System::power(bool reset, char fill_value)
{
    cpu.power(reset);
    ppu.power(reset);
    port.load(Controller::Type::Gamepad);
    std::fill(rammem.begin(), rammem.end(), fill_value);
    if (!reset) {
        std::fill(vrammem.begin(), vrammem.end(), fill_value);
        std::fill(palmem.begin(),  palmem.end(), fill_value);
    }
}

using DecodeFn = u16 (*)(u16);
static DecodeFn get_decode(Mirroring mirroring)
{
    switch (mirroring) {
    case Mirroring::OneScreen:  return [](u16 addr) -> u16 { return addr & util::bitmask(10); };
    case Mirroring::Vertical:   return [](u16 addr) -> u16 { return addr & util::bitmask(11); };
    case Mirroring::FourScreen: return [](u16 addr) -> u16 { return addr & util::bitmask(12); };
    case Mirroring::Horizontal:
        return [](u16 addr) -> u16 {
            auto tmp = addr & 0xFFF;
            auto bits = util::getbits(tmp, 10, 2) >> 1;
            return util::setbits(tmp, 10, 2, bits);
        };
    default:
        panic("invalid value passed to get_decode\n");
    }
}

void System::map(Mirroring mirroring)
{
    rambus.reset();
    vrambus.reset();
    rambus.map(RAM_START, PPUREG_START,     [this](u16 addr) { return rammem[addr & 0x7FF]; },       [this](u16 addr, u8 data) { rammem[addr & 0x7FF] = data; });
    rambus.map(APU_START, CARTRIDGE_START,  [this](u16 addr) { return cpu.readreg(addr); },          [this](u16 addr, u8 data) { cpu.writereg(addr, data); });
    rambus.map(PPUREG_START, APU_START,     [this](u16 addr) { return ppu.readreg(addr & 0x2007); }, [this](u16 addr, u8 data) { ppu.writereg(addr & 0x2007, data); });
    rambus.map(CARTRIDGE_START, 0x8000,     [this](u16 addr) { return mapper->read_wram(addr); },    [this](u16 addr, u8 data) { mapper->write_wram(addr, data); });
    rambus.map(0x8000, CPUBUS_SIZE,         [this](u16 addr) { return mapper->read_rom(addr); },     [this](u16 addr, u8 data) { mapper->write_rom(addr, data); });
    vrambus.map(PT_START, NT_START,         [this](u16 addr) { return mapper->read_chr(addr); },     [this](u16 addr, u8 data) { mapper->write_chr(addr, data); });
    vrambus.map(PAL_START, 0x4000,          [this](u16 addr) { return palmem[addr & 0x1F]; },        [this](u16 addr, u8 data) { palmem[addr & 0x1F] = data; });
    change_mirroring(mirroring);
}

void System::change_mirroring(Mirroring mirroring)
{
    const auto decode = get_decode(mirroring);
    vrambus.map(NT_START, PAL_START,        [this, decode](u16 addr) { return vrammem[decode(addr)]; }, [this, decode](u16 addr, u8 data) { vrammem[decode(addr)] = data; });
}

Emulator::Emulator()
{
    system.ppu.on_nmi([this](bool nmi_enabled) {
        nmi = true;
        program.video_frame(system.screen.data());
        if (nmi_enabled)
            system.cpu.fire_nmi();
    });
}

void System::run()
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
    while (!nmi && !stopped)
        system.run();
    nmi = false;
}

bool Emulator::insert_rom(const Cartridge::Data &cartdata)
{
    system.prgrom = cartdata.prgrom;
    system.chrrom = cartdata.chrrom;
    system.mapper = Mapper::create(cartdata.mapper, &system);
    if (system.mapper) {
        system.map(cartdata.mirroring);
        return true;
    }
    return false;
}

} // namespace core

#ifndef CORE_EMULATOR_HPP_INCLUDED
#define CORE_EMULATOR_HPP_INCLUDED

#include <emu/core/bus.hpp>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>
#include <fmt/core.h>

namespace Util { class File; }
namespace Debugger { class Debugger; }

namespace Core {

class Emulator {
    Bus<CPUBUS_SIZE> rambus;
    Bus<PPUBUS_SIZE> vrambus;
    Util::HeapArray<uint8> prgrom;
    Util::HeapArray<uint8> chrrom;
    CPU cpu{&rambus};
    PPU ppu{&vrambus};
    // this is internal to the emulator only and doesn't affect the cpu and ppu
    bool nmi = false;

    void bus_map(Mirroring mirroring);

public:
    Emulator()
    {
        ppu.on_nmi([this]() {
            nmi = true;
            cpu.fire_nmi();
        });
    }

    void power(bool reset = false);
    void run();
    void run_frame();
    void insert_rom(Cartridge::Data &&cartdata);

    uint32 *get_screen() { return ppu.get_screen(); }

    friend class Debugger::Debugger;
};

} // namespace Core

#endif

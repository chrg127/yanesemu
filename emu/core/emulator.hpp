#pragma once

#include <span>
#include <fmt/core.h>
#include <emu/core/bus.hpp>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/core/screen.hpp>
#include <emu/core/memory.hpp>

namespace Util { class File; }
namespace Debugger { class Debugger; }

namespace Core {

class Emulator {
    Bus<CPUBUS_SIZE> rambus;
    Bus<PPUBUS_SIZE> vrambus;
    Screen screen;
    std::span<uint8> prgrom;
    std::span<uint8> chrrom;
    CPU cpu{&rambus};
    PPU ppu{&vrambus, &screen};
    // this is internal to the emulator only and doesn't affect the cpu and ppu
    bool nmi = false;
    Memory memory;

    void bus_map(Mirroring mirroring);

public:
    Emulator()
    {
        ppu.on_nmi([this](bool nmi_enabled) {
            nmi = true;
            if (nmi_enabled)
                cpu.fire_nmi();
        });
    }

    void power(bool reset = false);
    void run();
    void run_frame();
    void insert_rom(Cartridge::Data &&cartdata);

    uint32 *get_screen()         { return screen.data(); }
    void on_cpu_error(auto &&fn) { cpu.on_error(fn); }

    friend class Debugger::Debugger;
};

} // namespace Core

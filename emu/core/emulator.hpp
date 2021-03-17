#ifndef CORE_EMULATOR_HPP_INCLUDED
#define CORE_EMULATOR_HPP_INCLUDED

#include <emu/core/bus.hpp>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/core/debugger.hpp>
#include <fmt/core.h>

namespace Util { class File; }

namespace Core {

class Emulator {
    Bus rambus { CPUBUS_SIZE };
    Bus vrambus { PPUBUS_SIZE };
    Cartridge cartridge;
    CPU cpu;
    PPU ppu;
    Debugger debugger {this};
    int cycle = 0;
    // this is internal to the emulator only and doesn't affect the cpu and ppu
    bool nmi = false;

public:
    Emulator()
    {
        cpu.attach_bus(&rambus);
        ppu.attach_bus(&vrambus, &rambus);
        ppu.set_nmi_callback([this]() {
            nmi = true;
            cpu.fire_nmi();
        });
    }

    void run();
    bool insert_rom(Util::File &romfile);
    std::string status() const;

    void power()
    {
        cpu.power();
        ppu.power();
    }

    void reset()
    {
        cpu.reset();
        ppu.reset();
    }

    void enable_debugger(auto &&callb)
    {
        // the debugger will do nothing as long we don't tie its run() function to something
        // when enabled, the debugger should stop at the first RESET.
        // this works as long Debugger::nextstop has a value.
        cpu.register_fetch_callback([&](uint16 addr, uint3 mode) { debugger.fetch_callback(addr, mode); });
        debugger.register_callback(callb);
    }

    void set_screen(Video::Canvas *canvas) { ppu.set_screen(canvas); }
    std::string rominfo()                  { return cartridge.getinfo(); }
    bool debugger_has_quit() const         { return debugger.has_quit(); }

    friend class Debugger;
};

} // namespace Core

#endif

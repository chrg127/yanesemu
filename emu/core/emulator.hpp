#ifndef CORE_EMULATOR_HPP_INCLUDED
#define CORE_EMULATOR_HPP_INCLUDED

#include <emu/core/bus.hpp>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <fmt/core.h>

namespace Util { class File; }
namespace Debugger { class Debugger; }

namespace Core {

class Emulator {
    Bus rambus { CPUBUS_SIZE };
    Bus vrambus { PPUBUS_SIZE };
    Util::HeapArray<uint8> prgrom;
    Util::HeapArray<uint8> chrrom;
    CPU cpu{&rambus};
    PPU ppu{&rambus, &vrambus};
    // this is internal to the emulator only and doesn't affect the cpu and ppu
    bool nmi = false;

public:
    Emulator()
    {
        ppu.attach_bus(&vrambus, &rambus);
        ppu.on_nmi([this]() {
            nmi = true;
            cpu.fire_nmi();
        });
    }

    void run();
    void run_frame();
    bool insert_rom(Util::File romfile);

    void power()
    {
        cpu.power();
        ppu.power();
    }

    void reset()
    {
        cpu.power(true);
        ppu.reset();
    }

    void set_screen(Video::Canvas *canvas) { ppu.set_screen(canvas); }
    // std::string rominfo()                  { return cartridge.getinfo(); }

    friend class Debugger::Debugger;
};

} // namespace Core

#endif

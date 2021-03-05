#ifndef CORE_EMULATOR_HPP_INCLUDED
#define CORE_EMULATOR_HPP_INCLUDED

#include <string_view>
#include <emu/core/bus.hpp>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>

namespace Util { class File; }

namespace Core {

class Emulator {
    Bus cpu_bus { CPUBUS_SIZE };
    Bus ppu_bus { PPUBUS_SIZE };
    Cartridge cartridge;
    CPU cpu;
    PPU ppu;
    int cycle = 0;
    // this is internal to the emulator only and doesn't affect the cpu and ppu
    bool nmi = false;

public:
    Emulator()
    {
        cpu.attach_bus(&cpu_bus);
        ppu.attach_bus(&ppu_bus, &cpu_bus);
        ppu.set_nmi_callback([this]() {
            nmi = true;
            cpu.fire_nmi();
        });
    }

    void run();
    void run_frame(Util::File &logfile);
    void log(Util::File &logfile);
    void dump(Util::File &dumpfile);
    void insert_rom(const std::string_view rompath);

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

    void set_screen(Video::Canvas *canvas) { ppu.set_screen(canvas); }
    std::string rominfo()                  { return cartridge.getinfo(); }
};

} // namespace Core

#endif

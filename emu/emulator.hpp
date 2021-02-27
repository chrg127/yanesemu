#ifndef EMULATOR_HPP_INCLUDED
#define EMULATOR_HPP_INCLUDED

#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>

namespace Util { class File; }

class Emulator {
    Core::Bus cpu_bus = Core::Bus(0x10000);
    Core::Bus ppu_bus = Core::Bus(0x4000);
    Core::Cartridge cartridge;
    Core::CPU cpu;
    Core::PPU ppu;
    int cycle = 0;
    int err = 0;

public:
    Emulator()
    {
        cpu.attach_bus(&cpu_bus);
        ppu.attach_bus(&ppu_bus, &cpu_bus);
    }

    void run();
    void wait_nmi();
    void log(Util::File &logfile);
    void dump(Util::File &dumpfile);

    void insert_rom(const std::string_view rompath)
    {
        cartridge.open(rompath);
        cartridge.attach_bus(&cpu_bus, &ppu_bus);
    }

    void power()
    {
        cpu.power();
        ppu.power();
    }

    void reset(const std::string_view rompath)
    {
        cpu.reset();
        ppu.reset();
    }

    void set_screen(Video::Canvas *canvas) { ppu.set_screen(canvas); }
    std::string rominfo()                  { return cartridge.getinfo(); }
};

#endif

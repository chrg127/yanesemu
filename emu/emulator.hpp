#ifndef EMULATOR_HPP_INCLUDED
#define EMULATOR_HPP_INCLUDED

#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/util/file.hpp>

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
    void insert_rom(const std::string_view rompath);
    void power();
    void run();
    void reset(const std::string_view rompath);
    void log(Util::File &logfile);
    void dump(Util::File &dumpfile);

    std::string rominfo() { return cartridge.getinfo(); }
};

#endif

#ifndef EMULATOR_HPP_INCLUDED
#define EMULATOR_HPP_INCLUDED

#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/util/file.hpp>

namespace Util { class File; }

class Emulator {
    Core::Cartridge cartridge;
    Core::CPU cpu;
    Core::PPU ppu;
    Core::Bus cpu_bus = Core::Bus(0x10000);
    Core::Bus ppu_bus = Core::Bus(0x4000);
    int cycle = 0;

public:
    bool init(std::string_view s, Util::File &f);
    void power();
    void run();
    void log(Util::File &logfile);
    void dump(Util::File &dumpfile);
};

#endif

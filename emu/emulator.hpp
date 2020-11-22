#ifndef EMULATOR_HPP_INCLUDED
#define EMULATOR_HPP_INCLUDED

#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/utils/file.hpp>

class Emulator {
    Core::CPU cpu;
    Core::PPU ppu;
    int cycle = 0;

public:
    void init(Core::Cartridge &cart);
    void run();
    void log(Util::File &logfile);
    void dump(Util::File &dumpfile);
};

#endif

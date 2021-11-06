#pragma once

#include <memory>
#include <span>
#include <emu/core/bus.hpp>
#include <emu/core/const.hpp>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/core/screen.hpp>
#include <emu/core/memory.hpp>
#include <emu/core/controller.hpp>
#include <emu/core/mapper.hpp>
#include <emu/util/uint.hpp>

namespace debugger { class Debugger; }

namespace core {

class Emulator {
    Bus<CPUBUS_SIZE> rambus;
    Bus<PPUBUS_SIZE> vrambus;
    ControllerPort port;
    Memory memory;
    Screen screen;
    CPU cpu{&rambus, &port};
    PPU ppu{&vrambus, &screen};
    std::span<u8> prgrom;
    std::span<u8> chrrom;
    std::unique_ptr<Mapper> mapper;
    // this is internal to the emulator only and doesn't affect the cpu and ppu
    bool nmi = false;
    bool emu_stop = false;

    void map(Mirroring mirroring);

public:
    Emulator();

    void power(bool reset = false);
    void run();
    void run_frame();
    bool insert_rom(const Cartridge::Data &cartdata);

    void connect_controller(Controller::Type type) { port.load(type); }
    void on_cpu_error(auto &&fn) { cpu.on_error(fn); }
    void change_mirroring(Mirroring mirroring) { memory.change_mirroring(mirroring, vrambus); }
    void stop()                  { emu_stop = true; }

    friend class debugger::Debugger;
};

extern Emulator emulator;

} // namespace core

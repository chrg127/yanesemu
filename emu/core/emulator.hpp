#pragma once

#include <array>
#include <memory>
#include <span>
#include <emu/core/bus.hpp>
#include <emu/core/const.hpp>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/core/screen.hpp>
#include <emu/core/controller.hpp>
#include <emu/core/mapper.hpp>
#include <emu/util/common.hpp>

namespace debugger { class Debugger; }

namespace core {

struct System {
    Bus<CPUBUS_SIZE> rambus;
    Bus<PPUBUS_SIZE> vrambus;
    Screen screen;
    ControllerPort port;
    CPU cpu{&rambus, &port};
    PPU ppu{&vrambus, &screen};
    std::unique_ptr<Mapper> mapper;
    std::span<u8> prgrom;
    std::span<u8> chrrom;
    std::array<u8, core::RAM_SIZE> rammem;
    std::array<u8, core::VRAM_SIZE> vrammem;
    std::array<u8, core::PAL_SIZE> palmem;
    int vram_id = 0;

    void run();
    void power(bool reset, char fill_value = 0);
    void map(Mirroring mirroring);
    void change_mirroring(Mirroring mirroring);
};

class Emulator {
    System system;
    bool nmi = false;
    bool stopped = false;

public:
    Emulator();

    bool insert_rom(const Cartridge::Data &cartdata);
    void run_frame();

    void power(bool reset = false)                 { system.power(reset); }
    void connect_controller(Controller::Type type) { system.port.load(type); }
    void on_cpu_error(auto &&fn)                   { system.cpu.on_error(fn); }
    void stop()                                    { stopped = true; }

    friend class debugger::Debugger;
};

extern Emulator emulator;

} // namespace core

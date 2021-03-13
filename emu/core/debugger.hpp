#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <vector>
#include <emu/core/cpu.hpp> // InstrInfo
#include <emu/util/unsigned.hpp>

namespace Core {

class Emulator;

class Debugger {
public:
    struct Event {
        uint16 pc;
        CPU::InstrInfo info;
    };

private:
    Emulator *emu;
    std::function<void (Debugger &, Event &)> callback;
    std::vector<uint16> breakpoints;
    bool quit = false;

public:
    Debugger(Emulator *e)
        : emu(e)
    { }

    void run(uint16 addr, uint3 mode);
    void step(Event &ev);
    // uint8 read_addr(uint16 addr);
    // void write_addr(uint16 addr, uint8 data);

    void register_callback(auto &&f) { callback = f; }
    void set_quit(bool q)       { quit = q; }
    bool has_quit() const       { return quit; }
};

void clirepl(Debugger &dbg, Debugger::Event &ev);

} // namespace Core

#endif

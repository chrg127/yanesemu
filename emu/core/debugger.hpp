#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <functional>
#include <vector>
#include <emu/core/opcodeinfo.hpp>
#include <emu/util/unsigned.hpp>

namespace Core {

class Emulator;

class Debugger {
public:
    struct Event {
        uint16 pc;
        Opcode opcode;
    };

    struct Breakpoint {
        uint16 start;
        uint16 end;
        uint3 mode; // rwx
    };

private:
    Emulator *emu;
    std::function<void (Debugger &, Event &)> callback;
    std::vector<Breakpoint> breakpoints;
    uint16 stop_addr = 0;
    bool quit = false;

public:
    Debugger(Emulator *e)
        : emu(e)
    { }

    void run(uint16 addr, uint3 mode);
    void set_stop_addr(uint16 addr) { stop_addr = addr; };
    void next(Opcode &op);
    void step(Opcode &op);
    // uint8 read_addr(uint16 addr);
    // void write_addr(uint16 addr, uint8 data);

    void register_callback(auto &&f) { callback = f; }
    void set_quit(bool q)       { quit = q; }
    bool has_quit() const       { return quit; }
};

void clirepl(Debugger &dbg, Debugger::Event &ev);

} // namespace Core

#endif

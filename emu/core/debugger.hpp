#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <functional>
#include <optional>
#include <vector>
#include <emu/core/opcodeinfo.hpp>
#include <emu/util/unsigned.hpp>

namespace Core {

class Emulator;

class Debugger {
public:
    struct Event {
        enum class Type      { FETCH, INTERRUPT, BREAK, };
        enum class Interrupt { RESET, IRQ, NMI, IGNORE, };
        Type type;
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
    std::optional<uint16> nextstop = 0;
    bool quit = false;

public:
    Debugger(Emulator *e)
        : emu(e)
    { }

    void fetch_callback(uint16 addr, uint3 mode);
    void set_nextstop(uint16 addr) { nextstop = addr; };
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

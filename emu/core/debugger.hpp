#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <functional>
#include <optional>
// #include <stack>
#include <vector>
#include <emu/core/opcodeinfo.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/util/circularbuffer.hpp>

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
    std::vector<Breakpoint> breakvec;
    std::optional<uint16> nextstop = 0;
    using TraceBuffer = std::vector<std::pair<uint16, Opcode>>;
    TraceBuffer tracebuf;
    bool quit = false;

public:
    Debugger(Emulator *e)
        : emu(e)
    { }

    void fetch_callback(uint16 addr, uint3 mode);
    void set_nextstop(uint16 addr) { nextstop = addr; };
    void next(Opcode &op);
    void step(Opcode &op);
    void continue_exec() { nextstop.reset(); }
    std::string regs() const;
    uint8 read(uint16 addr);
    void write(uint16 addr, uint8 value);
    void reset();

    void register_callback(auto &&f)            { callback = f; }
    void set_quit(bool q)                       { quit = q; }
    bool has_quit() const                       { return quit; }
    TraceBuffer tracebuffer() const             { return tracebuf; }
    void setbreak(Breakpoint &&p)               { breakvec.push_back(p); }
    void delbreak(unsigned index)               { breakvec.erase(breakvec.begin() + index); }
    std::vector<Breakpoint> breakpoints() const { return breakvec; }
};

void clirepl(Debugger &dbg, Debugger::Event &ev);

} // namespace Core

#endif

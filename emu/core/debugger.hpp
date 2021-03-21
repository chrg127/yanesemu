#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <functional>
#include <optional>
#include <vector>
#include <emu/core/opcodeinfo.hpp>

namespace Core {

class Emulator;

class Debugger {
public:
    struct Event {
        enum class Type { FETCH, BREAK, };
        Type type;
        uint16 pc;
        Opcode opcode;
        long index;
    };

    struct Breakpoint {
        uint16 start;
        uint16 end;
        char mode;
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

    void fetch_callback(uint16 addr, char mode);
    void set_nextstop(uint16 addr) { nextstop = addr; };
    void next(const Opcode &op);
    void step(const Opcode &op);
    void continue_exec() { nextstop.reset(); }
    std::string regs() const;
    uint8 read(uint16 addr);
    void write(uint16 addr, uint8 value);
    std::vector<uint8> readblock(uint16 start, uint16 end);
    void reset();

    void register_callback(auto &&f)            { callback = f; }
    void set_quit(bool q)                       { quit = q; }
    bool has_quit() const                       { return quit; }
    TraceBuffer tracebuffer() const             { return tracebuf; }
    unsigned setbreak(Breakpoint &&p)           { breakvec.push_back(p); return breakvec.size() - 1; }
    void delbreak(unsigned index)               { breakvec.erase(breakvec.begin() + index); }
    std::vector<Breakpoint> breakpoints() const { return breakvec; }
};

void clirepl(Debugger &dbg, Debugger::Event &ev);

} // namespace Core

#endif

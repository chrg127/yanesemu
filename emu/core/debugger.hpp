#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <functional>
#include <optional>
#include <vector>
#include <emu/core/instrinfo.hpp>

namespace Core {

class Emulator;

class Debugger {
public:
    struct Event {
        enum class Type { FETCH, BREAK, };
        Type type;
        Instruction instr;
        long index;
    };

    struct Breakpoint {
        uint16 start;
        uint16 end;
        char mode;
    };

    enum class StatusDev { CPU, PPU };

private:
    Emulator *emu;
    std::function<void (Debugger &, Event &)> callback;
    std::vector<Breakpoint> breakvec;
    std::optional<uint16> nextstop = 0;
    std::vector<Instruction> btrace;
    // bool tracing = false;
    bool quit = false;

public:
    Debugger(Emulator *e)
        : emu(e)
    { }

    void fetch_callback(uint16 addr, char mode);
    void next(const Instruction &op);
    void step(const Instruction &op);
    void continue_exec() { nextstop.reset(); }
    uint8 read(uint16 addr);
    void write(uint16 addr, uint8 value);
    std::string status(StatusDev dev) const;
    void reset();
    void enable_trace(std::string &&filename);

    void register_callback(auto &&f)            { callback = f; }
    void set_nextstop(uint16 addr)              { nextstop = addr; };
    void set_quit(bool q)                       { quit = q; }
    bool has_quit() const                       { return quit; }
    std::vector<Instruction> backtrace() const  { return btrace; }
    unsigned setbreak(Breakpoint &&p)           { breakvec.push_back(p); return breakvec.size() - 1; }
    void delbreak(unsigned index)               { breakvec.erase(breakvec.begin() + index); }
    std::vector<Breakpoint> breakpoints() const { return breakvec; }
};

void clirepl(Debugger &dbg, Debugger::Event &ev);

} // namespace Core

#endif

#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <functional>
#include <optional>
#include <vector>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/instrinfo.hpp>

namespace Core {

class Emulator;

class Debugger {
public:
    struct Event {
        enum class Type { FETCH, BREAK, };
        Type type;
        CPU::Status cpu_st;
        long index;
    };

    struct Breakpoint {
        uint16 start;
        uint16 end;
        char mode;
    };

private:
    Emulator *emu;
    std::function<void (Debugger &, Event &&)> callback;
    std::vector<Breakpoint> breakvec;
    std::optional<uint16> nextstop = 0;
    std::vector<CPU::Status> btrace;
    // bool tracing = false;
    bool quit = false;

public:
    Debugger(Emulator *e)
        : emu(e)
    { }

    void fetch_callback(CPU::Status &&st, uint16 addr, char mode);
    void next();
    void step();
    uint8 read(uint16 addr);
    void write(uint16 addr, uint8 value);
    CPU::Status cpu_status() const;
    PPU::Status ppu_status() const;

    void continue_exec()                        { nextstop.reset(); }
    void register_callback(auto &&f)            { callback = f; }
    void set_nextstop(uint16 addr)              { nextstop = addr; };
    void set_quit(bool q)                       { quit = q; }
    unsigned setbreak(Breakpoint &&p)           { breakvec.push_back(p); return breakvec.size() - 1; }
    void delbreak(unsigned index)               { breakvec.erase(breakvec.begin() + index); }
    bool has_quit() const                       { return quit; }
    std::vector<CPU::Status> backtrace() const  { return btrace; }
    std::vector<Breakpoint> breakpoints() const { return breakvec; }

private:
    void update_backtrace(CPU::Status st);
};

enum Command {
    HELP,
    CONTINUE,
    RUNFRAME,
    BREAK,
    LIST_BREAK,
    DELBREAK,
    BACKTRACE,
    NEXT,
    STEP,
    STATUS,
    READ_ADDR,
    WRITE_ADDR,
    BLOCK,
    DISASSEMBLE,
    DISBLOCK,
    RESET,
    QUIT,
    INVALID,
    LAST,
};

using CmdArgs = std::vector<std::string>;
class CliDebugger {
    Command cmd;
    CmdArgs cmdargs;
public:
    void repl(Debugger &dbg, Debugger::Event &&ev);
};


} // namespace Core

#endif

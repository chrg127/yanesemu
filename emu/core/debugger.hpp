#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <functional>
#include <optional>
#include <vector>
#include <emu/core/cpu.hpp>
#include <emu/core/ppu.hpp>
#include <emu/core/instrinfo.hpp>
#include <emu/util/file.hpp>

namespace Core {

class Emulator;

struct Debugger {
    struct Event {
        enum class Type { FETCH, BREAK, };
        Type type;
        CPU::Status cpu_st;
        long index;
    };
    using BreakCallback = std::function<void (Debugger &, Event &&)>;

    struct Breakpoint {
        uint16 start;
        uint16 end;
        char mode;
    };

private:
    Emulator *emu;
    BreakCallback callback;
    std::vector<Breakpoint> breakvec;
    std::optional<uint16> nextstop = 0;
    std::vector<CPU::Status> btrace;
    Util::File tracefile;
    bool quit = false;

public:
    explicit Debugger(Emulator *e) : emu(e) { }

    void fetch_callback(CPU::Status &&st, uint16 addr, char mode);
    void next();
    void step();
    uint8 read(uint16 addr);
    void write(uint16 addr, uint8 value);
    CPU::Status cpu_status() const;
    PPU::Status ppu_status() const;
    void start_tracing(Util::File &&f);

    void register_callback(auto &&f)            { callback = f; }
    void continue_exec()                        { nextstop.reset(); }
    void set_nextstop(uint16 addr)              { nextstop = addr; };
    void set_quit(bool q)                       { quit = q; }
    unsigned setbreak(Breakpoint &&p)           { breakvec.push_back(p); return breakvec.size() - 1; }
    void delbreak(unsigned index)               { breakvec.erase(breakvec.begin() + index); }
    void stop_tracing()                         { tracefile.close(); }
    bool has_quit() const                       { return quit; }
    std::vector<CPU::Status> backtrace() const  { return btrace; }
    std::vector<Breakpoint> breakpoints() const { return breakvec; }

private:
    void update_backtrace(CPU::Status st);
    void trace(CPU::Status &st, PPU::Status &&pst);
};

} // namespace Core

#endif

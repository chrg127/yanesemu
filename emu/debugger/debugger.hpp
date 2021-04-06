#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <functional>
#include <optional>
#include <vector>
#include <emu/util/unsigned.hpp>
#include <emu/util/file.hpp>

namespace Core {
    class Emulator;
    class CPU;
    class PPU;
}

namespace Debugger {

class CPUDebugger {
    Core::CPU *cpu;

public:
    CPUDebugger(Core::CPU *c)
        : cpu(c)
    { }

    enum class Reg {
        ACC, X, Y, PC, SP, FLAGS,
    };

    struct Instruction {
        uint8 id, lo, hi;
    };

    uint16 getreg(Reg reg) const;
    void setreg(Reg reg, uint16 value);
    Instruction curr_instr();
    std::string curr_instr_str();
    std::string curr_flags_str();
    unsigned long cycles();
};

class PPUDebugger {
    Core::PPU *ppu;

public:
    enum class Reg {
        CTRL, MASK, STATUS, OAMADDR, OAMDATA, PPUADDR, PPUSCROLL, PPUDATA,
    };

    struct Position {
        unsigned long cycle;
        unsigned long line;
    };

    uint16 getreg(Reg reg) const;
    void setreg(Reg reg, uint16 value);
    Position pos() const;
};

struct Debugger {
    struct Event {
        enum class Tag {
            STEP, BREAK, INV_INSTR,
        } tag;
        union {
            unsigned bp_index;
            struct {
                uint8 id;
                uint16 addr;
            } inv;
        };
    };

    struct Breakpoint {
        uint16 start;
        uint16 end;
        char mode;

        bool operator==(const Breakpoint &b) { return start == b.start && end == b.end && mode == b.mode; };
        bool test(uint16 addr, char mode) const { return this->mode == mode && addr >= start && addr <= end; };
    };

    enum class Step {
        STEP,
        NEXT,
        FRAME,
        NONE,
    };

private:
    Core::Emulator *emu;
    Step steptype = Step::NONE;
    std::vector<Breakpoint> breakvec;
    int break_hit = -1;
    Util::File tracefile;
    std::function<void(Event &&)> report_callback;

public:
    CPUDebugger cpudbg;
    PPUDebugger ppudbg;

    explicit Debugger(Core::Emulator *e);

    void on_report(auto &&f) { report_callback = f; }
    void run();

    void step();
    void next();
    void runframe();
    void continue_exec();

    uint8 readmem(uint16 addr);
    void writemem(uint16 addr, uint8 value);

    unsigned setbreak(Breakpoint &&p) { breakvec.push_back(p); return breakvec.size() - 1; }
    void delbreak(unsigned index)     { breakvec.erase(breakvec.begin() + index); }

    void start_tracing(Util::File &&f);
    void stop_tracing() { tracefile.close(); }

    std::vector<Breakpoint> breakpoints() const { return breakvec; }

private:
    void fetch_callback(uint16 addr, char mode);
    void error_callback(uint8 id, uint16 addr);
    void trace();
};

} // namespace Core

#endif

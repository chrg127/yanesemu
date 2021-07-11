#pragma once

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
        ACC, X, Y, PC, SP,
    };

    enum class Flag {
        NEG, OV, DEC, INTDIS, ZERO, CARRY,
    };

    struct Instruction {
        uint8 id, lo, hi;
    };

    uint16 getreg(Reg reg) const;
    bool getflag(Flag flag) const;
    void setreg(Reg reg, uint16 value);
    void setflag(Flag flag, bool value);
    uint16 get_vector_addr(uint16 vector);
    Instruction curr_instr();
    std::string curr_instr_str();
    std::string curr_flags_str();
    unsigned long cycles();
};

class PPUDebugger {
    Core::PPU *ppu;

public:
    enum class Reg {
        CTRL,
        MASK,
        STATUS,
        OAMADDR,
        OAMDATA,
        PPUADDR,
        PPUSCROLL,
        PPUDATA,
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

        // bool operator==(const Breakpoint &b) { return start == b.start && end == b.end && mode == b.mode; };
        bool test(uint16 addr, char mode) const { return this->mode == mode && addr >= start && addr <= end; };
    };

    enum class StepType {
        STEP, NEXT, FRAME, NONE,
    };

    enum class Loc {
        RAM, VRAM
    };

private:
    Core::Emulator *emu;
    std::function<void (Event &&)> report_callback;
    std::vector<Breakpoint> break_list;
    std::optional<Util::File> tracefile;
    bool got_error = false;

public:
    CPUDebugger cpudbg;
    PPUDebugger ppudbg;

    explicit Debugger(Core::Emulator *e);

    void on_report(auto &&f) { report_callback = f; }
    void run(StepType step_type);

    void step();
    void next();
    void advance();
    void advance_frame();

    uint8 readmem(uint16 addr, Loc loc);
    void writemem(uint16 addr, uint8 value, Loc loc);

    unsigned set_breakpoint(Breakpoint &&p);
    std::vector<Breakpoint> breakpoints() const { return break_list; }
    void delete_breakpoint(unsigned index)
    {
        break_list[index].mode = 'n';
    }

    bool start_tracing(std::string_view pathname)
    {
        stop_tracing();
        tracefile = Util::File::open(pathname, Util::Access::WRITE);
        return !!tracefile;
    }

    void stop_tracing() { tracefile = std::nullopt; }

private:
    void fetch_callback(uint16 addr, char mode);
    void error_callback(uint8 id, uint16 addr);
    int test_breakpoints();
    void trace();
};

} // namespace Core

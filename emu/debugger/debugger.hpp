#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <emu/util/uint.hpp>
#include <emu/util/file.hpp>

namespace core {
    class Emulator;
    class CPU;
    class PPU;
}

namespace Debugger {

enum class MemorySource {
    RAM,
    VRAM,
    OAM,
};

std::optional<MemorySource> string_to_memsource(const std::string &str);

class CPUDebugger {
    core::CPU *cpu;

public:
    CPUDebugger(core::CPU *c)
        : cpu(c)
    { }

    enum class Reg  { ACC, X, Y, PC, SP, };
    enum class Flag { NEG, OV, DEC, INTDIS, ZERO, CARRY, };

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
    core::PPU *ppu;

public:
    PPUDebugger(core::PPU *p) : ppu(p) {}

    enum class Reg {
        CTRL,    MASK,      STATUS,  OAMADDR,
        OAMDATA, PPUSCROLL, PPUADDR, PPUDATA,
    };

    uint8 getreg(uint16 addr) const;
    uint8 getreg(Reg reg) const;
    // void setreg(Reg reg, uint16 value);
    std::pair<unsigned long, unsigned long> pos() const;
    uint16 nt_base_addr() const;
    uint16 vram_addr() const;
    uint16 tmp_addr() const;
    uint8 fine_x() const;
    std::pair<int, int> screen_coords() const;
    uint8 read_oam(uint8 addr);
    void write_oam(uint8 addr, uint8 data);
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

private:
    core::Emulator *emu;
    std::function<void (Event &&)> report_callback;
    std::vector<Breakpoint> break_list;
    std::optional<io::File> tracefile;
    bool got_error = false;

public:
    CPUDebugger cpudbg;
    PPUDebugger ppudbg;

    explicit Debugger(core::Emulator *e);

    void on_report(auto &&f) { report_callback = f; }
    void run(StepType step_type);

    uint8 read_ram(uint16 addr);
    std::function<uint8(uint16)>       read_from(MemorySource source);
    std::function<void(uint16, uint8)> write_to(MemorySource source);

    void step()          { run(StepType::STEP); }
    void next()          { run(StepType::NEXT); }
    void advance()       { run(StepType::NONE); }
    void advance_frame() { run(StepType::FRAME); }

    unsigned set_breakpoint(Breakpoint &&p);
    void delete_breakpoint(unsigned index);
    std::vector<Breakpoint> breakpoints() const { return break_list; }

    bool start_tracing(std::string_view pathname);
    void stop_tracing();

private:
    void trace();
};

} // namespace core

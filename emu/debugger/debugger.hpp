#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <span>
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
    explicit CPUDebugger(core::CPU *c) : cpu(c) { }

    enum class Reg  { Acc, X, Y, PC, SP, };
    enum class Flag { Neg, Ov, Dec, IntDis, Zero, Carry, };

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
    explicit PPUDebugger(core::PPU *p) : ppu(p) {}

    enum class Reg {
        Ctrl,    Mask,      Status,  OAMAddr,
        OAMData, PPUScroll, PPUAddr, PPUData,
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
        enum class Type {
            Step, Break, InvalidInstruction,
        } type;
        union {
            unsigned point_num;
            struct {
                uint8 id;
                uint16 addr;
            } inv;
        };
    };

    struct Breakpoint {
        uint16 start;
        uint16 end;
        bool erased = false;
    };

    enum class StepType {
        Step, Next, Frame, None,
    };

private:
    core::Emulator *emu;
    std::function<void(Event)> report_callback;
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

    void step()          { run(StepType::Step); }
    void next()          { run(StepType::Next); }
    void advance()       { run(StepType::None); }
    void advance_frame() { run(StepType::Frame); }

    unsigned set_breakpoint(Breakpoint point);
    void delete_breakpoint(unsigned index);
    std::span<const Breakpoint> breakpoints() const { return break_list; }

    bool start_tracing(std::string_view pathname);
    void stop_tracing();

private:
    void trace();
};

} // namespace core

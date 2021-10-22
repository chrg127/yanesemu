#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <span>
#include <string_view>
#include <emu/util/uint.hpp>
#include <emu/util/file.hpp>

namespace core {
    class Emulator;
    class CPU;
    class PPU;
}

namespace debugger {

enum class MemorySource {
    RAM,
    VRAM,
    OAM,
};

class CPUDebugger {
    core::CPU *cpu;

public:
    explicit CPUDebugger(core::CPU *c) : cpu(c) { }

    enum class Reg  { Acc, X, Y, PC, SP, };
    enum class Flag { Neg, Ov, Dec, IntDis, Zero, Carry, };

    struct Instruction {
        u8 id, lo, hi;
    };

    u16 getreg(Reg reg) const;
    bool getflag(Flag flag) const;
    void setreg(Reg reg, u16 value);
    void setflag(Flag flag, bool value);
    u16 get_vector_addr(u16 vector) const;
    Instruction curr_instr() const;
    std::string curr_instr_str() const;
    std::string curr_flags_str() const;
    unsigned long cycles() const;
};

class PPUDebugger {
    core::PPU *ppu;

public:
    explicit PPUDebugger(core::PPU *p) : ppu(p) {}

    enum class Reg {
        Ctrl,    Mask,      Status,  OAMAddr,
        OAMData, PPUScroll, PPUAddr, PPUData,
    };

    u8 getreg(u16 addr) const;
    u8 getreg(Reg reg) const;
    // void setreg(Reg reg, u16 value);
    std::pair<unsigned long, unsigned long> pos() const;
    u16 nt_base_addr() const;
    u16 vram_addr() const;
    u16 tmp_addr() const;
    u8 fine_x() const;
    std::pair<int, int> screen_coords() const;
    u8 read_oam(u8 addr);
    void write_oam(u8 addr, u8 data);
};

struct Breakpoint {
    u16 start = 0, end = 0;
    bool erased = false;
};

class BreakList {
    std::vector<Breakpoint> list;
public:
    unsigned add(Breakpoint point);
    void erase(unsigned i) { list[i].erased = true; }
    auto begin() const     { return list.begin(); }
    auto end() const       { return list.end(); }
    auto size() const      { return list.size(); }
    auto & operator[](std::size_t i) const { return list[i]; }
    std::optional<unsigned> test(u16 addr);
};

struct Debugger {
    struct Event {
        enum class Type {
            Step, Break, InvalidInstruction,
        } type;
        union {
            unsigned point_id;
            struct {
                u8 id;
                u16 addr;
            } inv;
        };
    };

    enum class StepType {
        Step, Next, Frame, None,
    };

private:
    core::Emulator *emu;
    std::function<void(Event)> report_callback;
    BreakList break_list;
    std::optional<io::File> tracefile;
    bool got_error = false;

public:
    CPUDebugger cpudbg;
    PPUDebugger ppudbg;

    explicit Debugger(core::Emulator *e);

    void on_report(auto &&f) { report_callback = f; }
    void run(StepType step_type);

    u8 read_ram(u16 addr);
    std::function<u8(u16)>       read_from(MemorySource source);
    std::function<void(u16, u8)> write_to(MemorySource source);

    void step()          { run(StepType::Step); }
    void next()          { run(StepType::Next); }
    void advance()       { run(StepType::None); }
    void advance_frame() { run(StepType::Frame); }

    BreakList & breakpoints() { return break_list; }

    bool start_tracing(std::string_view pathname)
    {
        tracefile = io::File::open(pathname, io::Access::WRITE);
        return bool(tracefile);
    }

    void stop_tracing() { tracefile = std::nullopt; }

private:
    void trace();
};

std::optional<MemorySource> string_to_memsource(std::string_view str);
std::string disassemble(const u8 id, const u8 oplow, const u8 ophigh);
unsigned num_bytes(u8 id);

inline void disassemble_block(u16 start, u16 end, auto &&readval, auto &&process)
{
    while (start <= end) {
        u8 id   = readval(start);
        u8 low  = readval(start + 1);
        u8 high = readval(start + 2);
        process(start, disassemble(id, low, high));
        start += num_bytes(id);
    }
}

} // namespace core

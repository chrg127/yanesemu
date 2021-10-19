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
        uint8 id, lo, hi;
    };

    uint16 getreg(Reg reg) const;
    bool getflag(Flag flag) const;
    void setreg(Reg reg, uint16 value);
    void setflag(Flag flag, bool value);
    uint16 get_vector_addr(uint16 vector) const;
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

struct Breakpoint {
    uint16 start = 0, end = 0;
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
    std::optional<unsigned> test(uint16 addr);
};

struct Debugger {
    struct Event {
        enum class Type {
            Step, Break, InvalidInstruction,
        } type;
        union {
            unsigned point_id;
            struct {
                uint8 id;
                uint16 addr;
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

    uint8 read_ram(uint16 addr);
    std::function<uint8(uint16)>       read_from(MemorySource source);
    std::function<void(uint16, uint8)> write_to(MemorySource source);

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
std::string disassemble(const uint8 id, const uint8 oplow, const uint8 ophigh);
unsigned num_bytes(uint8 id);

inline void disassemble_block(uint16 start, uint16 end, auto &&readval, auto &&process)
{
    while (start <= end) {
        uint8 id   = readval(start);
        uint8 low  = readval(start + 1);
        uint8 high = readval(start + 2);
        process(start, disassemble(id, low, high));
        start += num_bytes(id);
    }
}

} // namespace core

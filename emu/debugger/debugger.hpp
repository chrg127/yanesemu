#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <span>
#include <string_view>
#include <utility>
#include <emu/util/uint.hpp>
#include <emu/util/file.hpp>

namespace core {
    class Emulator;
    class CPU;
    class PPU;
}
namespace input {
    enum class Button;
}

namespace debugger {

enum class MemorySource {
    RAM,
    VRAM,
    OAM,
};

enum class Component {
    CPU, PPU,
};

std::optional<MemorySource> string_to_memsource(std::string_view str);
std::optional<Component> string_to_component(std::string_view str);
std::optional<input::Button> string_to_button(std::string_view str);

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

class Tracer {
    std::optional<io::File> file;
public:
    bool start(std::string_view pathname)
    {
        file = io::File::open(pathname.data(), io::Access::WRITE);
        return bool(file);
    }
    void stop() { file = std::nullopt; }
    void trace(CPUDebugger &cpudbg, PPUDebugger &ppudbg);
};

class Debugger {
protected:
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
    bool got_error = false;

protected:
    CPUDebugger cpudbg;
    PPUDebugger ppudbg;
    BreakList break_list;
    Tracer tracer;

    Debugger();
    void on_report(auto &&f) { report_callback = f; }
    void run(StepType step_type);
    u8 read_ram(u16 addr);
    std::function<u8(u16)> read_from(MemorySource source);
    std::function<void(u16, u8)> write_to(MemorySource source);
    void reset_emulator();
};

std::pair<std::string, int> disassemble(const u8 id, const u8 oplow, const u8 ophigh);

inline void disassemble_block(u16 start, u16 end, auto &&readval, auto &&process)
{
    while (start <= end) {
        u8 id   = readval(start);
        u8 low  = readval(start + 1);
        u8 high = readval(start + 2);
        auto [str, num_bytes] = disassemble(id, low, high);
        process(start, str);
        start += num_bytes;
    }
}

} // namespace core

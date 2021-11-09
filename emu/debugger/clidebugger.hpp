#pragma once

#include <vector>
#include <string>
#include <emu/platform/input.hpp>
#include <emu/debugger/debugger.hpp>

namespace debugger {

class CliDebugger : public Debugger {
    std::vector<std::string> last_args{};

    void help();
    void breakpoint(u16 start, u16 end);
    void delete_breakpoint(int index);
    void list_breakpoints();
    void status(Component component);
    void read_block(u16 start, u16 end, MemorySource source);
    void write_addr(u16 addr, u8 data, MemorySource source);
    void write_block(u16 start, u16 end, u8 data, MemorySource source);
    void disassemble(u8 id, u8 low, u8 high);
    void disassemble_block(u16 start, u16 end);
    void trace(std::string_view filename);
    void stop_tracing();

    void breakpoint_single(u16 addr)              { breakpoint(addr, addr); }
    void read_addr(u16 addr, MemorySource source) { read_block(addr, addr, source); }
    void read_addr_ram(u16 addr)                  { read_block(addr, addr, MemorySource::RAM); }
    void read_block_ram(u16 start, u16 end)       { read_block(start, end, MemorySource::RAM); }
    void write_addr_ram(u16 addr, u8 data)        { write_addr(addr, data, MemorySource::RAM); }
    void write_cpu_reg(CPUDebugger::Reg reg, u16 data) { cpu.set_reg(reg, data); }
    void hold_button(input::Button button)        { Debugger::hold_button(button, true); }
    void unhold_button(input::Button button)      { Debugger::hold_button(button, false); }

    void report_event(Debugger::Event ev);
    void print_cpu_status() const;
    void print_ppu_status() const;

public:
    explicit CliDebugger()
    {
        on_report([&](Debugger::Event ev) { report_event(ev); });
    }

    bool repl();
    void print_instr() const;
};

}

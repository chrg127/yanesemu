#pragma once

#include <vector>
#include <string>
#include <emu/debugger/debugger.hpp>

namespace debugger {

class CliDebugger {
    Debugger dbg;
    std::string last_name;
    std::vector<std::string> last_args{};

    void help();
    void continue_exec();
    void runframe();
    void next();
    void step();
    void breakpoint_range(u16 start, u16 end);
    void breakpoint(u16 addr);
    void delete_break(int index);
    void list_breakpoints();
    void status(Component component);
    void status_cpu();
    void read_addr(u16 addr, MemorySource source);
    void read_addr_ram(u16 addr);
    void write_addr(u16 addr, u8 data, MemorySource source);
    void write_addr_ram(u16 addr, u8 data);
    void block(u16 start, u16 end, MemorySource source);
    void block_ram(u16 start, u16 end);
    void disassemble(u8 id, u8 low, u8 high);
    void disassemble_block(u16 start, u16 end);
    void trace(std::string_view filename);
    void stop_tracing();
    void reset();

    void report_event(Debugger::Event ev);
    void print_cpu_status() const;
    void print_ppu_status() const;

public:
    explicit CliDebugger()
    {
        dbg.on_report([this](Debugger::Event ev) { report_event(ev); });
    }

    bool repl();
    void print_instr() const;
};

}

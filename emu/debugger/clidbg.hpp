#pragma once

#include <vector>
#include <string>
#include <emu/debugger/debugger.hpp>

namespace core { class Emulator; }

namespace debugger {

enum CommandType {
    Help,        Continue,  RunFrame,   Next,
    Step,        Break,     ListBreaks, DeleteBreak,
    Status,      ReadAddr,  WriteAddr,  Block,
    Disassemble, DisBlock,  Trace,      StopTrace,
    WriteRegCPU, WriteRegPPU,
    HoldButton,  Reset,     Quit,
};

struct Command {
    CommandType type;
    std::string name;
    std::string desc;
    std::string abbrev;
    unsigned minargs;
    unsigned maxargs;
};

class CliDebugger : public Debugger {
    const Command *last_cmd = nullptr;
    std::vector<std::string> last_args{};
    bool quit = false;

public:
    CliDebugger();
    bool repl();
    void print_instr();

private:
    void eval(const Command &cmd, std::span<std::string> args);
    void report_event(Debugger::Event ev);
    void print_cpu_status() const;
    void print_ppu_status() const;
    void read_block(u16 start, u16 end, MemorySource source);
    void write_block(u16 start, u16 end, u8 data, MemorySource source);
};

} // namespace Debugger

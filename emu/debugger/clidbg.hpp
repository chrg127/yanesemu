#pragma once

#include <vector>
#include <string>
#include <emu/debugger/debugger.hpp>

namespace core { class Emulator; }

namespace Debugger {

enum Command {
    HELP,       CONTINUE,       RUNFRAME,   NEXT,
    STEP,       BREAK,          LIST_BREAK, DELBREAK,
    BACKTRACE,  STATUS,         READ_ADDR,  WRITE_ADDR,
    BLOCK,      DISASSEMBLE,    DISBLOCK,   TRACE,
    STOP_TRACE, RESET,          QUIT,
};

class CliDebugger {
    Debugger dbg;
    Command last_cmd;
    std::vector<std::string> last_args;
    bool quit = false;

public:
    explicit CliDebugger(core::Emulator *emu);
    bool repl();
    void print_instr();

private:
    void eval(Command cmd, std::vector<std::string> args);
    void report_event(Debugger::Event &&ev);
    void print_cpu_status();
    void print_ppu_status();
    std::string format_flags();
};

} // namespace Debugger

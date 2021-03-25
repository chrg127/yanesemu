#ifndef CORE_CLIDBG_HPP_INCLUDED
#define CORE_CLIDBG_HPP_INCLUDED

#include <vector>
#include <string>
#include <emu/core/debugger.hpp>

namespace Core {

enum Command {
    HELP,
    CONTINUE,
    RUNFRAME,
    BREAK,
    LIST_BREAK,
    DELBREAK,
    BACKTRACE,
    NEXT,
    STEP,
    STATUS,
    READ_ADDR,
    WRITE_ADDR,
    BLOCK,
    DISASSEMBLE,
    DISBLOCK,
    TRACE,
    STOP_TRACE,
    RESET,
    QUIT,
    INVALID,
    LAST,
};

using CmdArgs = std::vector<std::string>;
class CliDebugger {
    Command cmd;
    CmdArgs cmdargs;
public:
    void repl(Debugger &dbg, Debugger::Event &&ev);
};

} // namespace Core

#endif


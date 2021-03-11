#include <emu/core/debugger.hpp>

#include <string>
#include <unordered_map>
#include <emu/util/stringops.hpp>

namespace Core {

enum CmdName {
    HELP, BREAK, QUIT
};

struct Command {
    CmdName name;
    std::string desc;
};

static const std::unordered_map<std::string, Command> cmdmap = {
    { "help",   { CmdName::HELP,    "prints this help text"     } },
    { "break",  { CmdName::BREAK,   "set a breakpoint"          } },
    { "quit",   { CmdName::QUIT,    "quit running"              } },
};

void Debugger::run()
{
    if (parsing) {
        repl();
        return;
    }
    emu.run();
    /*
    // find if the CPU's current address has reached a breakpoint
    uint16 curr_addr = emu.cpu.pc.reg();
    if (auto it = breakpoints.find(); it != breakpoints.end()) {
        fmt::print("breakpoint #{} reached\n", it - breakpoints.begin());
    }
    */
}

void Debugger::repl()
{
    fmt::print("> ");
    std::string line = "";
    if (!input.getline(line)) {
        quit = true;
        return;
    }
    auto tokens = Util::strsplit(line);
    auto it = cmdmap.find(tokens[0]);
    if (it == cmdmap.end()) {
        fmt::print("{} is not a valid command. See 'help' for a list of commands.\n", tokens[0]);
        return;
    }
    CmdName cmd = it->second.name;
    switch (cmd) {
    case CmdName::HELP:
        print_help();
        break;
    case CmdName::BREAK:
        break;
    case CmdName::QUIT:
        quit = true;
        break;
    }
}

void Debugger::print_help()
{
    for (const auto &cmd : cmdmap) {
        fmt::print("{}: {}\n", cmd.first, cmd.second.desc);
    }
}

} // namespace Core


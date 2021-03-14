#include <emu/core/debugger.hpp>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <emu/core/emulator.hpp>
#include <emu/util/stringops.hpp>
#include <emu/util/file.hpp>

namespace Core {

// Debugger functions
void Debugger::run(uint16 addr, uint3 mode)
{
    Event ev;
    ev.info = emu->cpu.disassemble();
    ev.pc = emu->cpu.pc.reg;
    if (mode == 0b001 && stop_addr == addr) {
        stop_addr = 0;
        callback(*this, ev);
    }
}

void Debugger::next(Event &ev)
{

}

void Debugger::step(Event &ev)
{
    static int branch_instr[] = { 0x10, 0x30, 0x50, 0x70, 0x90, 0xB0, 0xD0 };
    if (
    stop_addr = ev.pc + ev.info.num_bytes;
}

/* The CLI debugger is actually a free function.
 * It has a lot of private data here. */
enum Command {
    HELP,
    BREAK,
    // WATCH,
    LIST_BREAK,
    // LIST_WATCH,
    BACKTRACE,
    CONTINUE,
    DISASSEMBLE,
    NEXT,
    STEP,
    STEP_OUT,
    STATUS,
    REGS,
    READ_ADDR,
    WRITE_ADDR,
    RESET,
    QUIT,
    INVALID,
};

#define O(X) \
    X("help",           "h",    Command::HELP,          "prints this help text") \
    X("break",          "b",    Command::BREAK,         "set a breakpoint") \
    X("listb",          "lb",   Command::LIST_BREAK,    "list breakpoints") \
    X("backtrace",      "bt",   Command::BACKTRACE,     "prints the backtrace") \
    X("continue",       "c",    Command::CONTINUE,      "start/continue execution") \
    X("disassemble",    "dis",  Command::DISASSEMBLE,   "disassemble the current instruction") \
    X("next",           "n",    Command::NEXT,          "run next instruction") \
    X("step",           "s",    Command::STEP,          "step next instruction") \
    X("stepout",        "so",   Command::STEP_OUT,      "step out next instruction") \
    X("status",         "st",   Command::STATUS,        "print machine status") \
    X("regs",           "r",    Command::REGS,          "print registers") \
    X("read",           "rd",   Command::READ_ADDR,     "read address") \
    X("write",          "wr",   Command::WRITE_ADDR,    "write address") \
    X("reset",          "res",  Command::RESET,         "reset the machine") \
    X("quit",           "q",    Command::QUIT,          "quit the emulator") \

#define X(name, abbrev, enumname, desc) { name, enumname }, { abbrev, enumname },
static const std::unordered_map<std::string, Command> strtocmd = {
    O(X)
};
#undef X

struct CommandInfo {
    std::string name;
    std::string desc;
    std::string abbrev;
};

#define X(name, abbrev, enumname, desc) { name, desc, abbrev },
static const std::vector<CommandInfo> cmdinfo = {
    O(X)
};
#undef X

static Command parse()
{
    Util::File input{stdin};
    std::string line = "";
    if (!input.getline(line))
        return Command::QUIT;
    auto tokens = Util::strsplit(line);
    if (tokens.size() == 0)
        return Command::INVALID;
    auto it = strtocmd.find(tokens[0]);
    if (it == strtocmd.end())
        return Command::INVALID;
    return it->second;
}

void clirepl(Debugger &dbg, Debugger::Event &ev)
{
    fmt::print("{:04X}: {}\n", ev.pc, ev.info.to_str);
    bool exit = false;
    do {
        fmt::print("> ");
        switch (parse()) {
        case Command::HELP:
            for (const auto &cmd : cmdinfo)
                fmt::print("{}, {}: {}\n", cmd.name, cmd.abbrev, cmd.desc);
            break;
        case Command::BREAK:
            break;
        case Command::LIST_BREAK:
            break;
        case Command::BACKTRACE:
            break;
        case Command::CONTINUE:
            fmt::print("continuing.\n");
            exit = true;
            break;
        case Command::DISASSEMBLE:
            break;
        case Command::NEXT:
            dbg.next(ev);
            break;
        case Command::STEP:
            dbg.step(ev);
            exit = true;
            break;
        case Command::STEP_OUT:
            break;
        case Command::STATUS:
            // fmt::print("{}\n", dbg.emu->status());
            break;
        case Command::REGS:
            break;
        case Command::READ_ADDR:
            // fmt::print("{}\n", dbg.read_addr(0);
            break;
        case Command::WRITE_ADDR:
            // dbg.write_addr(0);
            break;
        case Command::RESET:
            break;
        case Command::QUIT:
            dbg.set_quit(true);
            exit = true;
            break;
        case Command::INVALID:
            fmt::print("invalid command. see 'help' for a list of commands.\n");
            break;
        }
    } while (!exit);
}

} // namespace Core


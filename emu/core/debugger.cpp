#include <emu/core/debugger.hpp>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <emu/core/emulator.hpp>
#include <emu/util/stringops.hpp>
#include <emu/util/file.hpp>

namespace Core {

void Debugger::fetch_callback(uint16 addr, uint3 mode)
{
    if (quit)
        return;
    Event ev;
    ev.pc = emu->cpu.pc.reg;
    if (nextstop) {
        /* If the user has issued a next/step command, but an interrupt
         * happened, modify the next stop address so that we get inside
         * the interrupt handler.
         * The interrupt vector is read 2 times, so make sure we ignore
         * the second read (can be done by checking addr) */
        if (mode == 0b100 && addr >= 0xFFFA && (addr & 1) == 0) {
            nextstop = emu->rambus.read(addr+1) << 8 | emu->rambus.read(addr);
        } else if (mode == 0b001 && addr == nextstop.value()) {
            nextstop.reset();
            ev.type   = Event::Type::FETCH;
            ev.opcode = emu->cpu.disassemble();
            callback(*this, ev);
        }
    }
}

void Debugger::step(Opcode &op)
{
    nextstop = emu->cpu.nextaddr(op);
}

void Debugger::next(Opcode &op)
{
    // check for jumps and skip them, otherwise use the usual function
    nextstop = is_jump(op.code) ? emu->cpu.pc.reg + op.info.numb
                                 : emu->cpu.nextaddr(op);
}

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
    fmt::print("{:04X}: {}\n", ev.pc, ev.opcode.info.str);
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
            dbg.next(ev.opcode);
            exit = true;
            break;
        case Command::STEP:
            dbg.step(ev.opcode);
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


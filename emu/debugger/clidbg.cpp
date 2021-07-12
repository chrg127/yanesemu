#include <emu/debugger/clidbg.hpp>

#include <unordered_map>
#include <fmt/core.h>
#include <emu/core/const.hpp>
#include <emu/core/instrinfo.hpp>
#include <emu/debugger/debugger.hpp>
#include <emu/util/unsigned.hpp>
#include <emu/util/stringops.hpp>
#include <emu/util/file.hpp>
#include <emu/util/stlutil.hpp>

namespace Debugger {

//  name                abbrev  enum           min args max args    description
#define O(X) \
    X("help",           "h",    HELP,          0,       1,          "prints this help text") \
    X("continue",       "c",    CONTINUE,      0,       0,          "start/continue execution") \
    X("runframe",       "nmi",  RUNFRAME,      0,       0,          "run entire frame, stop at nmi handler") \
    X("break",          "b",    BREAK,         2,       3,          "set a breakpoint") \
    X("listbreaks",     "lb",   LIST_BREAK,    0,       0,          "list breakpoints") \
    X("deletebreak",    "dlb",  DELBREAK,      1,       1,          "delete a breakpoint") \
    X("backtrace",      "bt",   BACKTRACE,     0,       0,          "prints the backtrace") \
    X("next",           "n",    NEXT,          0,       0,          "run next instruction") \
    X("step",           "s",    STEP,          0,       0,          "step next instruction") \
    X("status",         "st",   STATUS,        0,       1,          "print current status") \
    X("read",           "rd",   READ_ADDR,     1,       1,          "read address") \
    X("write",          "wr",   WRITE_ADDR,    2,       2,          "write address") \
    X("block",          "bl",   BLOCK,         2,       3,          "read block") \
    X("disassemble",    "dis",  DISASSEMBLE,   1,       3,          "disassemble the current instruction") \
    X("disblock",       "disb", DISBLOCK,      2,       2,          "disassemble a given block") \
    X("trace",          "tr",   TRACE,         1,       1,          "") \
    X("stoptrace",      "str",  STOP_TRACE,    0,       0,          "") \
    X("reset",          "res",  RESET,         0,       0,          "reset the machine") \
    X("quit",           "q",    QUIT,          0,       0,          "quit the emulator") \

#define X(name, abbrev, enumname, minargs, maxargs, desc) { name, enumname }, { abbrev, enumname },
static const std::unordered_map<std::string, Command> name_lookup = {
    O(X)
};
#undef X

struct CommandInfo {
    std::string name;
    std::string desc;
    std::string abbrev;
    unsigned minargs;
    unsigned maxargs;
};

#define X(name, abbrev, enumname, minargs, maxargs, desc) { enumname, CommandInfo{ name, desc, abbrev, minargs, maxargs } },
static const std::unordered_map<Command, CommandInfo> info_lookup = {
    O(X)
};
#undef X

static const std::string helpstr = R"(help, h:            prints this help text
continue, c:        start/continue execution
runframe, nmi:      run entire frame, stop at nmi handler
break, b:           set a breakpoint
listbreaks, lb:     list breakpoints
deletebreak, dlb:   delete a breakpoint
backtrace, bt:      prints the backtrace
next, n:            run next instruction
step, s:            step next instruction
status, st:         print current status
read, rd:           read address
write, wr:          write address
block, bl:          read block
disassemble, dis:   disassemble the current instruction
disblock, disb:     disassemble a given block
trace, tr:          enable tracing
stoptrace, str:     stop tracing
reset, res:         reset the machine
quit, q:            quit the emulator
)";

// Prints a whole block. Start and end are both inclusive (to allow reads of
// 0000-FFFF).
static void print_block(uint16 start, uint16 end, auto &&readvalue)
{
    for (int i = 0; i <= (end - start); ) {
        fmt::print("{:04X}: ", i + start);
        for (int j = 0; j < 16 && i + start <= end; i++, j++)
            fmt::print("{:02X} ", readvalue(i + start));
        fmt::print("\n");
    }
}

static void block_command(const std::vector<std::string> &args, auto &&process)
{
    auto getloc = [](const std::string &arg) -> Debugger::Loc
    {
        if (arg == "cpu") return Debugger::Loc::RAM;
        if (arg == "ppu") return Debugger::Loc::VRAM;
        fmt::print("Unrecognized location: {}\n", arg);
        return Debugger::Loc::RAM;
    };

    auto start = Util::strconv<uint16>(args[0], 16);
    auto end   = Util::strconv<uint16>(args[1], 16);
    Debugger::Loc loc = args.size() == 3 ? getloc(args[2]) : Debugger::Loc::RAM;
    if (!start || !end)
        fmt::print("Invalid value found while parsing arguments.\n");
    else if (end.value() <= start.value())
        fmt::print("Invalid range.\n");
    else
        process(start.value(), end.value(), loc);
}

CliDebugger::CliDebugger(Core::Emulator *emu)
    : dbg(emu)
{
    dbg.on_report([this](Debugger::Event &&ev) { report_event(std::move(ev)); });
}

// void CliDebugger::enter()
// {
//     print_instr();
//     repl();
// }

bool CliDebugger::repl()
{
    Util::File input = Util::File::assoc(stdin);
    std::string cmdstr, argsstr;

    fmt::print("[NESDBG]> ");
    if (!input.getword(cmdstr) || !input.getline(argsstr)) {
        quit = true;
        return quit;
    }
    if (cmdstr.empty())
        eval(last_cmd, last_args);
    else if (auto optcmd = Util::map_lookup(name_lookup, cmdstr); optcmd) {
        last_cmd  = optcmd.value();
        last_args = Util::strsplit(argsstr, ' ');
        eval(last_cmd, last_args);
    } else
        fmt::print("Invalid command. Try 'help'.\n");

    return quit;
}

void CliDebugger::eval(Command cmd, std::vector<std::string> args)
{
    const auto &info = info_lookup.find(cmd)->second;
    if (args.size() < info.minargs ) {
        fmt::print("Not enough arguments for command {}. Try 'help'.\n", info.name);
        return;
    } else if (args.size() > info.maxargs) {
        fmt::print("Too many arguments for command {}. Try 'help'.\n", info.name);
        return;
    }

    switch (cmd) {

    case Command::HELP:
        if (args.size() == 1) {
            auto cmd = Util::map_lookup(name_lookup, args[0]);
            fmt::print("{}\n", info_lookup.find(cmd.value())->second.desc);
        } else
            fmt::print("{}", helpstr);
        break;

    case Command::CONTINUE:
        fmt::print("Continuing.\n");
        dbg.advance();
        break;

    case Command::RUNFRAME:
        dbg.advance_frame();
        break;

    case Command::NEXT:
        dbg.next();
        break;

    case Command::STEP:
        dbg.step();
        break;

    case Command::BREAK: {
        char mode = args[0][0];
        if (args[0].size() != 1 || (mode != 'r' && mode != 'w' && mode != 'x')) {
            fmt::print("Invalid mode for breakpoint. Try 'help'.\n");
            break;
        }
        auto start = Util::strconv<uint16>(args[1], 16);
        auto end = args.size() > 2 ? Util::strconv<uint16>(args[2], 16) : start;
        if (!start || !end || end.value() < start.value()) {
            fmt::print("Invalid range.\n");
            break;
        }
        unsigned i = dbg.set_breakpoint({ .start = start.value(), .end = end.value(), .mode = mode });
        fmt::print("Set breakpoint #{} to {:04X}-{:04X}.\n", i, start.value(), end.value());
        break;
    }

    case Command::DELBREAK: {
        auto index = Util::strconv<uint16>(args[0], 16);
        if (!index || index >= dbg.breakpoints().size())
            fmt::print("Index not valid.\n");
        else {
            dbg.delete_breakpoint(index.value());
            fmt::print("Breakpoint #{} deleted.\n", index.value());
        }
        break;
    }

    case Command::LIST_BREAK: {
        const auto &breaks = dbg.breakpoints();
        for (std::size_t i = 0; i < breaks.size(); i++) {
            if (breaks[i].mode == 'n')
                continue;
            fmt::print("#{}: {:04X}-{:04X}, mode: {}\n",
                    i, breaks[i].start, breaks[i].end, breaks[i].mode);
        }
        break;
    }

    case Command::BACKTRACE: {
    }

    case Command::DISASSEMBLE: {
        auto id = Util::strconv<uint8>(args[0], 16);
        auto lo = args.size() >= 2 ? Util::strconv<uint16>(args[1], 16) : 0;
        auto hi = args.size() >= 3 ? Util::strconv<uint16>(args[2], 16) : 0;
        if (!id || !lo || !hi)
            fmt::print("Invalid value found while parsing command arguments.\n");
        else
            fmt::print("{}\n", Core::disassemble(id.value(), lo.value(), hi.value()));
        break;
    }

    case Command::STATUS:
        if (!args.empty() && args[0] == "ppu")
            print_ppu_status();
        else
            print_cpu_status();
        break;

    case Command::READ_ADDR: {
        auto addr = Util::strconv<uint16>(args[0], 16);
        if (!addr)
            fmt::print("Invalid value found while parsing command arguments.\n");
        else
            fmt::print("{:02X}\n", dbg.readmem(addr.value(), Debugger::Loc::RAM));
        break;
    }

    case Command::WRITE_ADDR: {
        auto addr = Util::strconv<uint16>(args[0], 16);
        auto val  = Util::strconv<uint8 >(args[1], 16);
        if (!addr || !val)
            fmt::print("Invalid value found while parsing command arguments.\n");
        else {
            if (addr.value() >= Core::CARTRIDGE_START)
                fmt::print("Warning: writes to ROM have no effects\n");
            dbg.writemem(addr.value(), val.value(), Debugger::Loc::RAM);
        }
        break;
    }

    case Command::BLOCK:
        block_command(args, [&](uint16 start, uint16 end, Debugger::Loc loc) {
            print_block(start, end, [&](uint16 addr) { return dbg.readmem(addr, loc); });
        });
        break;

    case Command::DISBLOCK:
        block_command(args, [&](uint16 start, uint16 end, Debugger::Loc loc) {
            Core::disassemble_block(start, end,
                              [&](uint16 addr) { return dbg.readmem(addr, Debugger::Loc::RAM); },
                              [ ](uint16 addr, std::string &&s) { fmt::print("${:04X}: {}\n", addr, s); });
        });
        break;

    case Command::TRACE: {
        if (!dbg.start_tracing(args[0]))
            std::perror("error");
        break;
    }

    case Command::STOP_TRACE:
        dbg.stop_tracing();
        break;

    case Command::RESET:
        break;

    case Command::QUIT:
        quit = true;
        break;
    }
}

void CliDebugger::report_event(Debugger::Event &&ev)
{
    switch (ev.tag) {
    case Debugger::Event::Tag::STEP:
        print_instr();
        break;
    case Debugger::Event::Tag::BREAK:
        fmt::print("Breakpoint #{} reached.\n", ev.bp_index);
        print_instr();
        break;
    case Debugger::Event::Tag::INV_INSTR:
        fmt::print("Found invalid instruction {:02X} at {:04X}.\n",
                   ev.inv.id, ev.inv.addr);
        break;
    }
}

void CliDebugger::print_instr()
{
    fmt::print("${:04X}: {}\n", dbg.cpudbg.getreg(CPUDebugger::Reg::PC),
                                dbg.cpudbg.curr_instr_str());
}

void CliDebugger::print_cpu_status()
{
    fmt::print("PC: ${:02X} A: ${:02X} X: ${:02X} Y: ${:02X} S: ${:02X}\n"
               "Flags: [{}]\n"
               "Cycles: {}\n",
               dbg.cpudbg.getreg(CPUDebugger::Reg::PC),
               dbg.cpudbg.getreg(CPUDebugger::Reg::ACC),
               dbg.cpudbg.getreg(CPUDebugger::Reg::X),
               dbg.cpudbg.getreg(CPUDebugger::Reg::Y),
               dbg.cpudbg.getreg(CPUDebugger::Reg::SP),
               dbg.cpudbg.curr_flags_str(),
               dbg.cpudbg.cycles());
}

void CliDebugger::print_ppu_status()
{
    fmt::print("Line: {} Cycle: {}\nVRAM Address: {:04X} TMP Address: {:04X}\n",
               0, 0, 0, 0);
}

} // namespace Debugger


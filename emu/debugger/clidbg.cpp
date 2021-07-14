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
#include <emu/util/debug.hpp>

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
    X("read",           "rd",   READ_ADDR,     1,       2,          "read address") \
    X("write",          "wr",   WRITE_ADDR,    2,       3,          "write address") \
    X("block",          "bl",   BLOCK,         2,       3,          "read block") \
    X("disassemble",    "dis",  DISASSEMBLE,   1,       3,          "disassemble the current instruction") \
    X("disblock",       "db",   DISBLOCK,      2,       2,          "disassemble a given block") \
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

static void read_block(Debugger &dbg, uint16 start, uint16 end, Debugger::Loc loc)
{
    if (end < start) {
        fmt::print("Invalid range.");
        return;
    }
    if (loc == Debugger::Loc::VRAM && (start > 0x4000 || end > 0x4000)) {
        fmt::print("Invalid range for VRAM.\n");
        return;
    }

    const std::unordered_map<Debugger::Loc, std::function<uint8(uint16)>> fnmap = {
        { Debugger::Loc::RAM,  Util::member_fn(&dbg, &Debugger::read_ram)  },
        { Debugger::Loc::VRAM, Util::member_fn(&dbg, &Debugger::read_vram) },
    };
    auto readfn = Util::map_lookup(fnmap, loc).value();

    for (int i = 0; i <= (end - start); ) {
        fmt::print("{:04X}: ", start + i);
        for (int j = 0; j < 16 && i + start <= end; i++, j++)
            fmt::print("{:02X} ", readfn(start + i));
        fmt::print("\n");
    }
}

static void write_block(Debugger &dbg, uint16 start, uint16 end, uint8 data, Debugger::Loc loc)
{
    const std::unordered_map<Debugger::Loc, std::function<void(uint16, uint8)>> fnmap = {
        { Debugger::Loc::RAM,  Util::member_fn(&dbg, &Debugger::write_ram)  },
        { Debugger::Loc::VRAM, Util::member_fn(&dbg, &Debugger::write_vram) },
    };
    auto writefn = Util::map_lookup(fnmap, loc).value();
    for (uint16 curr = start; curr <= (end-start); curr++)
        writefn(curr, data);
}

static std::optional<uint16> parse_addr(const std::string &str)
{
    auto addr = Util::strconv<uint16>(str, 16);
    if (!addr) fmt::print("Invalid address: {}\n", str);
    return addr;
}

static std::optional<uint8> parse_data(const std::string &str)
{
    auto data = Util::strconv<uint8>(str, 8);
    if (!data) fmt::print("Invalid value: {}\n", str);
    return data;
}

static std::optional<Debugger::Loc> parse_mem_loc(const std::string &arg)
{
    std::unordered_map<std::string, Debugger::Loc> locmap = {
        { "",    Debugger::Loc::RAM },
        { "cpu", Debugger::Loc::RAM },
        { "ppu", Debugger::Loc::VRAM },
    };
    auto loc = Util::map_lookup(locmap, arg);
    if (!loc) fmt::print("Unrecognized memory source: {}\n", arg);
    return loc;
}



CliDebugger::CliDebugger(Core::Emulator *emu)
    : dbg(emu)
{
    dbg.on_report([this](Debugger::Event &&ev) { report_event(std::move(ev)); });
}

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
            auto cmd = Util::map_lookup_withdef(name_lookup, args[0], Command::HELP);
            fmt::print("{}\n", info_lookup.find(cmd)->second.desc);
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
        auto start = parse_addr(args[1]);
        auto end = args.size() > 2 ? parse_addr(args[2]) : start;
        if (start && end) {
            if (end.value() < start.value())
                fmt::print("Invalid range.\n");
            else {
                unsigned i = dbg.set_breakpoint({ .start = start.value(), .end = end.value(), .mode = mode });
                fmt::print("Set breakpoint #{} to {:04X}-{:04X}.\n", i, start.value(), end.value());
            }
        }
        break;
    }

    case Command::DELBREAK: {
        auto index = Util::strconv(args[0]);
        if (!index || index >= dbg.breakpoints().size())
            fmt::print("Invalid index: {}.\n", args[0]);
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
        auto id = parse_data(args[0]);
        auto lo = args.size() >= 2 ? parse_data(args[1]) : 0;
        auto hi = args.size() >= 3 ? parse_data(args[2]) : 0;
        if (id && lo && hi)
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
        auto addr = parse_addr(args[0]);
        auto loc = parse_mem_loc(args.size() == 2 ? args[1] : "");
        if (addr && loc)
            read_block(dbg, addr.value(), addr.value(), loc.value());
        break;
    }

    case Command::WRITE_ADDR: {
        auto addr = parse_addr(args[0]);
        auto val  = parse_data(args[1]);
        auto loc  = parse_mem_loc(args.size() == 3 ? args[2] : "");
        if (addr && val && loc) {
            if (loc.value() == Debugger::Loc::RAM && addr.value() >= Core::CARTRIDGE_START)
                fmt::print("Warning: writes to ROM have no effects\n");
            write_block(dbg, addr.value(), addr.value(), val.value(), loc.value());
            // dbg.write_ram(addr.value(), val.value());
        }
        break;
    }

    case Command::BLOCK: {
        auto start = parse_addr(args[0]);
        auto end = parse_addr(args[1]);
        auto loc = parse_mem_loc(args.size() == 3 ? args[2] : "");
        if (start && end && loc)
            read_block(dbg, start.value(), end.value(), loc.value());
        break;
    }

    case Command::DISBLOCK: {
        auto start = parse_addr(args[0]);
        auto end   = parse_addr(args[1]);
        if (start && end) {
            if (end.value() < start.value())
                fmt::print("Invalid range.\n");
            else
                Core::disassemble_block(start.value(), end.value(),
                    [&](uint16 addr)                  { return dbg.read_ram(addr); },
                    [ ](uint16 addr, std::string &&str) { fmt::print("${:04X}: {}\n", addr, str); });
        }
        break;
    }

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


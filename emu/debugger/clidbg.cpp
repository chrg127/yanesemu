#include <emu/debugger/clidbg.hpp>

#include <unordered_map>
#include <fmt/core.h>
#include <emu/core/const.hpp>
#include <emu/core/instrinfo.hpp>
#include <emu/debugger/debugger.hpp>
#include <emu/util/bits.hpp>
#include <emu/util/uint.hpp>
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

static bool check_addr_ranges(uint16 start, uint16 end, Debugger::Loc loc)
{
    if (end < start) {
        fmt::print("Invalid range: {:04X}-{:04X}.", start, end);
        return false;
    } else if (loc == Debugger::Loc::VRAM && (start > 0x4000 || end > 0x4000)) {
        fmt::print("Invalid range for VRAM.\n");
        return false;
    } else if (loc == Debugger::Loc::OAM && (start > 0xFF || end > 0xFF)) {
        fmt::print("Invalid range for source OAM.\n");
        return false;
    }
    return true;
}

static void read_block(Debugger &dbg, uint16 start, uint16 end, Debugger::Loc loc)
{
    if (!check_addr_ranges(start, end, loc))
        return;
    auto readfn = get_read_fn(loc);
    for (int i = 0; i <= (end - start); ) {
        fmt::print("${:04X}: ", start + i);
        for (int j = 0; j < 16 && i + start <= end; i++, j++)
            fmt::print("{:02X} ", readfn(&dbg, start + i));
        fmt::print("\n");
    }
}

static void write_block(Debugger &dbg, uint16 start, uint16 end, uint8 data, Debugger::Loc loc)
{
    if (!check_addr_ranges(start, end, loc))
        return;
    auto writefn = get_write_fn(loc);
    for (uint16 curr = start; curr <= (end-start); curr++)
        writefn(&dbg, curr, data);
}

static std::optional<uint16> parse_addr(const std::string &str)
{
    auto addr = str::conv<uint16>(str, 16);
    if (!addr) fmt::print("Invalid address: {}\n", str);
    return addr;
}

static std::optional<uint8> parse_data(const std::string &str)
{
    auto data = str::conv<uint8>(str, 8);
    if (!data) fmt::print("Invalid value: {}\n", str);
    return data;
}

static std::optional<Debugger::Loc> parse_mem_loc(const std::string &str)
{
    auto loc = str_to_memsrc(str);
    if (!loc) fmt::print("Unrecognized memory source: {}\n", str);
    return loc;
}

CliDebugger::CliDebugger(core::Emulator *emu)
    : dbg(emu)
{
    dbg.on_report([this](Debugger::Event &&ev) { report_event(std::move(ev)); });
}

bool CliDebugger::repl()
{
    io::File input = io::File::assoc(stdin);
    std::string cmdstr, argsstr;

    fmt::print("[NESDBG]> ");
    if (!input.getword(cmdstr) || !input.getline(argsstr)) {
        quit = true;
        return quit;
    }
    if (cmdstr.empty())
        eval(last_cmd, last_args);
    else if (auto optcmd = util::map_lookup(name_lookup, cmdstr); optcmd) {
        last_cmd  = optcmd.value();
        last_args = str::split(argsstr, ' ');
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
            auto cmd = util::map_lookup_withdef(name_lookup, args[0], Command::HELP);
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
        auto index = str::conv(args[0]);
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
            fmt::print("{}\n", core::disassemble(id.value(), lo.value(), hi.value()));
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
            if (loc.value() == Debugger::Loc::RAM && addr.value() >= core::CARTRIDGE_START)
                fmt::print("Warning: writes to ROM have no effects\n");
            write_block(dbg, addr.value(), addr.value(), val.value(), loc.value());
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
        if (start && end && check_addr_ranges(start.value(), end.value(), Debugger::Loc::RAM)) {
            core::disassemble_block(start.value(), end.value(),
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
    using util::getbit;
    auto onoff = [](auto val) { return val ? "ON" : "OFF"; };
    uint8 ctrl = dbg.ppudbg.getreg(PPUDebugger::Reg::CTRL);
    fmt::print("PPUCTRL ($2000): {:08b}:\n"
               "    Base NT address: ${:04X}\n    VRAM address increment: {}\n    Sprite Pattern table address: ${:04X}\n"
               "    Background pattern table address: ${:04X}\n    Sprite size: {}\n    Master/slave: {}\n    NMI enabled: {}\n",
               ctrl, dbg.ppudbg.nt_base_addr(), (ctrl & 4) == 0 ? 1 : 32, getbit(ctrl, 4) * 0x1000,
               getbit(ctrl, 5) * 0x1000, (ctrl & 32) ? "8x16" : "8x8", (ctrl & 64) ? "output color" : "read backdrop",
               (ctrl & 128) ? "ON" : "OFF");
    uint8 mask = dbg.ppudbg.getreg(PPUDebugger::Reg::MASK);
    fmt::print("PPUMASK ($2001): {:08b}:\n"
               "    Greyscale: {}\n    BG left: {}\n    Sprites left: {}\n    BG: {}\n    Sprites: {}\n"
               "    Emphasize red: {}\n    Emphasize green: {}\n    Emphasize blue: {}\n",
               mask, onoff(mask & 1),  onoff(mask & 2),  onoff(mask & 4),  onoff(mask & 8),
                     onoff(mask & 16), onoff(mask & 32), onoff(mask & 64), onoff(mask & 128));
    uint8 status = dbg.ppudbg.getreg(PPUDebugger::Reg::STATUS);
    fmt::print("PPUSTATUS ($2002): {:08b}:\n"
               "    Sprite overflow: {}\n    Sprite 0 hit: {}\n    Vblank: {}\n",
               status, onoff(status & 32), onoff(status & 64), onoff(status & 128));
    fmt::print("OAMADDR ($2003): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::OAMADDR));
    fmt::print("OAMDATA ($2004): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::OAMDATA));
    fmt::print("PPUSCROLL ($2005): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::PPUSCROLL));
    fmt::print("PPUADDR ($2006): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::PPUADDR));
    fmt::print("PPUDATA ($2007): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::PPUDATA));
    auto [l, c] = dbg.ppudbg.pos();
    fmt::print("Line: {}; Cycle: {}\n", l, c);
    fmt::print("VRAM address: {:04X}\n", dbg.ppudbg.vram_addr());
    fmt::print("TMP address: {:04X}\n", dbg.ppudbg.tmp_addr());
    fmt::print("Fine X: {:X}\n", dbg.ppudbg.fine_x());
    auto [x, y] = dbg.ppudbg.screen_coords();
    fmt::print("Screen coordinates: X = {}, Y = {}\n", x, y);
}

} // namespace Debugger


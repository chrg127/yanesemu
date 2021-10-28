#include <emu/debugger/clidbg.hpp>

#include <unordered_map>
#include <stdexcept>
#include <fmt/core.h>
#include <emu/core/const.hpp>
#include <emu/debugger/debugger.hpp>
#include <emu/debugger/instrinfo.hpp>
#include <emu/util/bits.hpp>
#include <emu/util/uint.hpp>
#include <emu/util/string.hpp>
#include <emu/util/file.hpp>
#include <emu/util/utility.hpp>
#include <emu/util/debug.hpp>

namespace debugger {

//  name                abbrev  enum           min args max args    description
#define ENUM_COMMANDS(X) \
    X("help",           "h",    Help,          0,       1,          "prints this help text") \
    X("continue",       "c",    Continue,      0,       0,          "start/continue execution") \
    X("runframe",       "nmi",  RunFrame,      0,       0,          "run entire frame, stop at nmi handler") \
    X("break",          "b",    Break,         1,       2,          "set a breakpoint") \
    X("listbreaks",     "lb",   ListBreaks,    0,       0,          "list breakpoints") \
    X("deletebreak",    "dlb",  DeleteBreak,   1,       1,          "delete a breakpoint") \
    X("next",           "n",    Next,          0,       0,          "run next instruction") \
    X("step",           "s",    Step,          0,       0,          "step next instruction") \
    X("status",         "st",   Status,        0,       1,          "print current status") \
    X("read",           "rd",   ReadAddr,      1,       2,          "read address") \
    X("write",          "wr",   WriteAddr,     2,       3,          "write address") \
    X("block",          "bl",   Block,         2,       3,          "read block") \
    X("disassemble",    "dis",  Disassemble,   1,       3,          "disassemble the current instruction") \
    X("disblock",       "db",   DisBlock,      2,       2,          "disassemble a given block") \
    X("trace",          "tr",   Trace,         1,       1,          "trace and log instructions to file") \
    X("stoptrace",      "str",  StopTrace,     0,       0,          "stop tracing instructions") \
    X("reset",          "res",  Reset,         0,       0,          "reset the emulator") \
    X("quit",           "q",    Quit,          0,       0,          "quit the emulator") \

#define X(name, abbrev, enumname, minargs, maxargs, desc) \
    { name,   { CommandType::enumname, name, desc, abbrev, minargs, maxargs } }, \
    { abbrev, { CommandType::enumname, name, desc, abbrev, minargs, maxargs } },
static const std::unordered_map<std::string, Command> commands = {
        ENUM_COMMANDS(X)
};
#undef X

static std::string get_helpstr()
{
#define X(name, abbrev, enumname, minargs, maxargs, desc) { CommandType::enumname, name, desc, abbrev, minargs, maxargs },
    static const std::vector<Command> v = { ENUM_COMMANDS(X) };
#undef X
    auto it = std::max_element(v.begin(), v.end(), [](const auto &a, const auto &b) {
        return a.name.size() + a.abbrev.size() < b.name.size() + b.abbrev.size();
    });
    auto size = it->name.size() + it->abbrev.size() + 3;
    std::string result;
    for (const auto &entry : v) {
        auto tmp = fmt::format("{}, {}:", entry.name, entry.abbrev);
        result += fmt::format("{:{}} {}\n", tmp, size, entry.desc);
    }
    return result;
}

static const std::string helpstr = get_helpstr();

struct CommandError : std::runtime_error {
    using std::runtime_error::runtime_error;
    using std::runtime_error::what;
};



namespace {

void check_addr_ranges(u16 start, u16 end, MemorySource source)
{
    if (end < start)
        throw CommandError(fmt::format("Invalid range: {:04X}-{:04X}.", start, end));
    if (source == MemorySource::VRAM && (start > 0x4000 || end > 0x4000))
        throw CommandError(fmt::format("Invalid range for source VRAM."));
    if (source == MemorySource::OAM && (start > 0xFF || end > 0xFF))
        throw CommandError(fmt::format("Invalid range for source OAM."));
}

void read_block(Debugger &dbg, u16 start, u16 end, MemorySource source)
{
    check_addr_ranges(start, end, source);
    auto readfn = dbg.read_from(source);
    for (int i = 0; i <= (end - start); ) {
        fmt::print("${:04X}: ", start + i);
        for (int j = 0; j < 16 && i + start <= end; i++, j++)
            fmt::print("{:02X} ", readfn(start + i));
        fmt::print("\n");
    }
}

void write_block(Debugger &dbg, u16 start, u16 end, u8 data, MemorySource source)
{
    check_addr_ranges(start, end, source);
    auto writefn = dbg.write_to(source);
    for (u16 curr = start; curr <= (end - start); curr++)
        writefn(curr, data);
}

template <typename T>
T parse_to(std::string_view str)
{
    std::optional<T> o;
    if constexpr(std::is_same<T, MemorySource>::value)
        o = string_to_memsource(str);
    else
        o = str::conv<T>(str, 16);
    if (!o) {
        if constexpr(std::is_same<T, MemorySource>::value) throw CommandError(fmt::format("Invalid memory source: {}", str));
        if constexpr(std::is_same<T, u8>::value)        throw CommandError(fmt::format("Invalid data: {}", str));
        if constexpr(std::is_same<T, u16>::value)       throw CommandError(fmt::format("Invalid address: {}", str));
    }
    return o.value();
}

} // namespace



CliDebugger::CliDebugger()
{
    dbg.on_report([this](Debugger::Event ev) { report_event(ev); });
    // last_cmd = commands.find("help")->second;
}

bool CliDebugger::repl()
{
    auto input = io::File::assoc(stdin);
    std::string cmdstr, argsstr;

    fmt::print(">>> ");
    if (!input.getword(cmdstr) || !input.getline(argsstr))
        return quit = true;

    try {
        if (!cmdstr.empty()) {
            if (auto cmd = commands.find(cmdstr); cmd != commands.end()) {
                last_cmd  = &cmd->second;
                last_args = str::split(argsstr, ' ');
            } else
                throw CommandError("Invalid command. Try 'help'.\n");
        }
        eval(*last_cmd, last_args);
    } catch (const CommandError &error) {
        fmt::print(stderr, "{}\n", error.what());
    }

    return quit;
}

void CliDebugger::eval(const Command &cmd, std::span<std::string> args)
{
    if (args.size() < cmd.minargs ) {
        fmt::print("Not enough arguments for command {}. Try 'help'.\n", cmd.name);
        return;
    } else if (args.size() > cmd.maxargs) {
        fmt::print("Too many arguments for command {}. Try 'help'.\n", cmd.name);
        return;
    }

    switch (cmd.type) {

    case CommandType::Help:
        if (args.size() == 1) {
            auto arg = util::map_lookup(commands, args[0]);
            if (!arg)
                fmt::print("not a command: {}\n", args[0]);
            else
                fmt::print("{}\n", arg.value().desc);
        } else
            fmt::print("{}", helpstr);
        break;

    case CommandType::Continue:
        fmt::print("Continuing.\n");
        dbg.advance();
        break;

    case CommandType::RunFrame:
        dbg.advance_frame();
        break;

    case CommandType::Next:
        dbg.next();
        break;

    case CommandType::Step:
        dbg.step();
        break;

    case CommandType::Break: {
        auto start = parse_to<u16>(args[0]);
        auto end = args.size() > 1 ? parse_to<u16>(args[1]) : start;
        if (end < start)
            throw CommandError(fmt::format("Invalid range."));
        else {
            unsigned i = dbg.breakpoints().add({ .start = start, .end = end });
            fmt::print("Set breakpoint #{} to {:04X}-{:04X}.\n", i, start, end);
        }
        break;
    }

    case CommandType::DeleteBreak: {
        auto index = str::conv(args[0]);
        if (!index || index >= dbg.breakpoints().size())
            throw CommandError(fmt::format("Invalid index: {}.", args[0]));
        else {
            dbg.breakpoints().erase(index.value());
            fmt::print("Breakpoint #{} deleted.\n", index.value());
        }
        break;
    }

    case CommandType::ListBreaks: {
        const auto &list = dbg.breakpoints();
        for (std::size_t i = 0; i < list.size(); i++) {
            if (list[i].erased)
                continue;
            fmt::print("#{}: {:04X}-{:04X}\n", i, list[i].start, list[i].end);
        }
        break;
    }

    case CommandType::Disassemble: {
        auto id = parse_to<u8>(args[0]);
        auto lo = args.size() >= 2 ? parse_to<u8>(args[1]) : 0;
        auto hi = args.size() >= 3 ? parse_to<u8>(args[2]) : 0;
        if (id && lo && hi)
            fmt::print("{}\n", disassemble(id, lo, hi).first);
        break;
    }

    case CommandType::Status:
        if (!args.empty() && args[0] == "ppu")
            print_ppu_status();
        else
            print_cpu_status();
        break;

    case CommandType::ReadAddr: {
        auto addr   = parse_to<u16>(args[0]);
        auto source = parse_to<MemorySource>(args.size() == 2 ? args[1] : "");
        read_block(dbg, addr, addr, source);
        break;
    }

    case CommandType::WriteAddr: {
        auto addr   = parse_to<u16>(args[0]);
        auto val    = parse_to<u8>(args[1]);
        auto source = parse_to<MemorySource>(args.size() == 3 ? args[2] : "");
        if (source == MemorySource::RAM && addr >= core::CARTRIDGE_START)
            fmt::print("Warning: writes to ROM have no effects\n");
        write_block(dbg, addr, addr, val, source);
        break;
    }

    case CommandType::Block: {
        auto start = parse_to<u16>(args[0]);
        auto end   = parse_to<u16>(args[1]);
        auto loc   = parse_to<MemorySource>(args.size() == 3 ? args[2] : "");
        read_block(dbg, start, end, loc);
        break;
    }

    case CommandType::DisBlock: {
        auto start = parse_to<u16>(args[0]);
        auto end   = parse_to<u16>(args[1]);
        check_addr_ranges(start, end, MemorySource::RAM);
        disassemble_block(start, end,
            [&](u16 addr)                       { return dbg.read_ram(addr); },
            [ ](u16 addr, std::string_view str) { fmt::print("${:04X}: {}\n", addr, str); });
        break;
    }

    case CommandType::Trace: {
        if (!dbg.start_tracing(args[0]))
            std::perror("error");
        break;
    }

    case CommandType::StopTrace:
        dbg.stop_tracing();
        break;

    case CommandType::Reset:
        break;

    case CommandType::Quit:
        quit = true;
        break;
    }
}

void CliDebugger::report_event(Debugger::Event ev)
{
    switch (ev.type) {
    case Debugger::Event::Type::Step:
        print_instr();
        break;
    case Debugger::Event::Type::Break:
        fmt::print("Breakpoint #{} reached.\n", ev.point_id);
        print_instr();
        break;
    case Debugger::Event::Type::InvalidInstruction:
        fmt::print("Found invalid instruction {:02X} at {:04X}.\n", ev.inv.id, ev.inv.addr);
        break;
    }
}

void CliDebugger::print_instr()
{
    fmt::print("${:04X}: {}\n", dbg.cpudbg.getreg(CPUDebugger::Reg::PC),
                                dbg.cpudbg.curr_instr_str());
}

void CliDebugger::print_cpu_status() const
{
    fmt::print("PC: ${:02X} A: ${:02X} X: ${:02X} Y: ${:02X} S: ${:02X}\n"
               "Flags: [{}]\n"
               "Cycles: {}\n",
               dbg.cpudbg.getreg(CPUDebugger::Reg::PC),
               dbg.cpudbg.getreg(CPUDebugger::Reg::Acc),
               dbg.cpudbg.getreg(CPUDebugger::Reg::X),
               dbg.cpudbg.getreg(CPUDebugger::Reg::Y),
               dbg.cpudbg.getreg(CPUDebugger::Reg::SP),
               dbg.cpudbg.curr_flags_str(),
               dbg.cpudbg.cycles());
}

void CliDebugger::print_ppu_status() const
{
    using util::getbit;
    auto onoff = [](auto val) { return val ? "ON" : "OFF"; };
    u8 ctrl = dbg.ppudbg.getreg(PPUDebugger::Reg::Ctrl);
    fmt::print("PPUCTRL ($2000): {:08b}:\n"
               "    Base NT address: ${:04X}\n    VRAM address increment: {}\n    Sprite Pattern table address: ${:04X}\n"
               "    Background pattern table address: ${:04X}\n    Sprite size: {}\n    Master/slave: {}\n    NMI enabled: {}\n",
               ctrl, dbg.ppudbg.nt_base_addr(), (ctrl & 4) == 0 ? 1 : 32, getbit(ctrl, 4) * 0x1000,
               getbit(ctrl, 5) * 0x1000, (ctrl & 32) ? "8x16" : "8x8", (ctrl & 64) ? "output color" : "read backdrop",
               (ctrl & 128) ? "ON" : "OFF");
    u8 mask = dbg.ppudbg.getreg(PPUDebugger::Reg::Mask);
    fmt::print("PPUMASK ($2001): {:08b}:\n"
               "    Greyscale: {}\n    BG left: {}\n    Sprites left: {}\n    BG: {}\n    Sprites: {}\n"
               "    Emphasize red: {}\n    Emphasize green: {}\n    Emphasize blue: {}\n",
               mask, onoff(mask & 1),  onoff(mask & 2),  onoff(mask & 4),  onoff(mask & 8),
                     onoff(mask & 16), onoff(mask & 32), onoff(mask & 64), onoff(mask & 128));
    u8 status = dbg.ppudbg.getreg(PPUDebugger::Reg::Status);
    fmt::print("PPUSTATUS ($2002): {:08b}:\n"
               "    Sprite overflow: {}\n    Sprite 0 hit: {}\n    Vblank: {}\n",
               status, onoff(status & 32), onoff(status & 64), onoff(status & 128));
    fmt::print("OAMAddr ($2003): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::OAMAddr));
    fmt::print("OAMData ($2004): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::OAMData));
    fmt::print("PPUScroll ($2005): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::PPUScroll));
    fmt::print("PPUAddr ($2006): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::PPUAddr));
    fmt::print("PPUData ($2007): ${:02X}\n", dbg.ppudbg.getreg(PPUDebugger::Reg::PPUData));
    auto [l, c] = dbg.ppudbg.pos();
    fmt::print("Line: {}; Cycle: {}\n", l, c);
    fmt::print("VRAM address: {:04X}\n", dbg.ppudbg.vram_addr());
    fmt::print("TMP address: {:04X}\n", dbg.ppudbg.tmp_addr());
    fmt::print("Fine X: {:X}\n", dbg.ppudbg.fine_x());
}

} // namespace Debugger

#include <emu/core/debugger.hpp>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <cassert>
#include <emu/core/emulator.hpp>
#include <emu/util/stringops.hpp>
#include <emu/util/file.hpp>

namespace Core {

void Debugger::fetch_callback(uint16 addr, char mode)
{
    if (quit)
        return;

    Instruction instr = emu->cpu.disassemble();

    // tracing
    if (mode == 'x') {
        switch (instr.id) {
        case 0x20: case 0x4C: case 0x6C: // jsr, jmp, jmp (ind)
            btrace.push_back(instr);
            break;
        case 0x60: // rts
            if (btrace.back().id == 0x20)
                btrace.pop_back();
            break;
        }
    }

    // check if we've reached a breakpoint
    auto it = std::find_if(breakvec.begin(), breakvec.end(), [=](const Breakpoint &p) {
                return p.mode == mode && addr >= p.start && addr <= p.end;
            });
    if (it != breakvec.end()) {
        Event ev = {
            .type   = Event::Type::BREAK,
            .instr  = instr,
            .index  = it - breakvec.begin(),
        };
        callback(*this, ev);
    }

    // handle next/step commands
    if (nextstop) {
        /* If the user has issued a next/step command, but an interrupt happened,
         * modify the next stop address so that we get inside the interrupt handler.
         * The interrupt vector is read 2 times, so make sure we ignore the second read */
        if (mode == 'r' && addr >= 0xFFFA && (addr & 1) == 0)
            nextstop = emu->rambus.read(addr+1) << 8 | emu->rambus.read(addr);
        else if (mode == 'x' && addr == nextstop.value()) {
            nextstop.reset();
            Event ev = {
                .type   = Event::Type::FETCH,
                .instr  = instr,
                .index  = 0,
            };
            callback(*this, ev);
        }
    }
}

void Debugger::step(const Instruction &instr)
{
    nextstop = emu->cpu.nextaddr(instr);
}

void Debugger::next(const Instruction &instr)
{
    // check for jumps and skip them, otherwise use the usual function
    nextstop = is_jump(instr.id) ? emu->cpu.pc.reg + instr.numb
                                 : emu->cpu.nextaddr(instr);
}

std::string Debugger::status(StatusDev dev) const
{
    switch (dev) {
    case StatusDev::CPU: return emu->cpu.status();
    case StatusDev::PPU: return emu->ppu.status();
    default: assert(false);
    }
}

void Debugger::reset()
{
}

uint8 Debugger::read(uint16 addr)
{
    /* PPU regs 2002 and 2007 have side effects, so emulate them.
     * for 2007, we always return the read buffer. */
    switch (addr) {
    case 0x2002: return emu->ppu.io.vblank << 7 | emu->ppu.io.sp_zero_hit << 6 | emu->ppu.io.sp_overflow << 5;
    case 0x2007: return emu->ppu.vram.addr < 0x3F00 ? emu->ppu.io.data_buf
                                                    : emu->vrambus.read(emu->ppu.vram.addr);
    default: return emu->rambus.read(addr);
    }
}

void Debugger::write(uint16 addr, uint8 value)
{
    emu->rambus.write(addr, value);
}



struct CommandInfo {
    std::string name;
    std::string desc;
    std::string abbrev;
    unsigned minargs;
};

//  name                abbrev  enum           min args    description
#define O(X) \
    X("help",           "h",    HELP,          0,          "prints this help text") \
    X("continue",       "c",    CONTINUE,      0,          "start/continue execution") \
    X("runframe",       "nmi",  RUNFRAME,      0,          "run entire frame, stop at nmi handler") \
    X("break",          "b",    BREAK,         2,          "set a breakpoint") \
    X("listbreaks",     "lb",   LIST_BREAK,    0,          "list breakpoints") \
    X("deletebreak",    "dlb",  DELBREAK,      1,          "delete a breakpoint") \
    X("backtrace",      "bt",   BACKTRACE,     0,          "prints the backtrace") \
    X("next",           "n",    NEXT,          0,          "run next instruction") \
    X("step",           "s",    STEP,          0,          "step next instruction") \
    X("status",         "st",   STATUS,        0,          "print current status") \
    X("read",           "rd",   READ_ADDR,     1,          "read address") \
    X("write",          "wr",   WRITE_ADDR,    2,          "write address") \
    X("block",          "bl",   BLOCK,         2,          "read block") \
    X("disassemble",    "dis",  DISASSEMBLE,   1,          "disassemble the current instruction") \
    X("disblock",       "disb", DISBLOCK,      2,          "disassemble a given block") \
    X("reset",          "res",  RESET,         0,          "reset the machine") \
    X("quit",           "q",    QUIT,          0,          "quit the emulator") \

#define X(name, abbrev, enumname, minargs, desc) enumname,
enum Command {
    O(X)
    INVALID,
};
#undef X

#define X(name, abbrev, enumname, minargs, desc) { name, enumname }, { abbrev, enumname },
static const std::unordered_map<std::string, Command> cmd_strtab = {
    O(X)
};
#undef X

#define X(name, abbrev, enumname, minargs, desc) { enumname, CommandInfo{ name, desc, abbrev, minargs } },
static std::unordered_map<Command, CommandInfo> cmd_infotab = {
    O(X)
};
#undef X

#define X(name, abbrev, enumname, minargs, desc) name ", " abbrev ": " desc "\n"
static const std::string cmd_helpstr = ""
    O(X)
    "";
#undef X

using CmdArgs = std::vector<std::string>;

static std::pair<Command, CmdArgs> parse()
{
    Util::File input{stdin};
    std::string cmd, args;

    if (!input.getword(cmd) || !input.getline(args))
        return std::make_pair(Command::QUIT, CmdArgs{});
    if (cmd.empty())
        return std::make_pair(Command::INVALID, CmdArgs{});
    auto it = cmd_strtab.find(cmd);
    if (it == cmd_strtab.end())
        return std::make_pair(Command::INVALID, CmdArgs{});
    return std::make_pair(it->second, Util::strsplit(args, ' '));
}

static void print_block(auto &&readvalue, uint16 start, uint16 end)
{
    for (uint16 i = 0; i < (end - start); ) {
        fmt::print("{:04X}: ", i + start);
        for (uint16 j = 0; j < 16 && i + start < end; j++) {
            fmt::print("{:02X} ", readvalue(i));
            i++;
        }
        fmt::print("\n");
    }
}

/* execute a command and return whether to quit the repl. */
static bool exec_command(Debugger &dbg, const Command cmd, const CmdArgs &args, const Instruction &instr)
{
    switch (cmd) {

    case Command::HELP:
        fmt::print("{}", cmd_helpstr);
        return false;

    case Command::CONTINUE:
        fmt::print("Continuing.\n");
        dbg.continue_exec();
        return true;

    case Command::RUNFRAME:
        dbg.set_nextstop(dbg.read(NMI_VEC) | dbg.read(NMI_VEC+1) << 8);
        return true;

    case Command::BREAK: {
        char mode = args[0][0];
        if (args[0].size() != 1 || (mode != 'r' && mode != 'w' && mode != 'x')) {
            fmt::print("Invalid mode for breakpoint. Try 'help'.\n");
            return false;
        }
        auto start = Util::strconv<uint16>(args[1], 16);
        auto end = args.size() > 2 ? Util::strconv<uint16>(args[2], 16) : start;
        if (!start || !end || end <= start) {
            fmt::print("Invalid range.\n");
            return false;
        }
        unsigned i = dbg.setbreak({ .start = start.value(), .end = end.value(), .mode = mode });
        fmt::print("Set breakpoint {} to {:04X}-{:04X}.\n", i, start.value(), end.value());
        return false;
    }

    case Command::DELBREAK: {
        auto index = Util::strconv<uint16>(args[0], 16);
        if (!index || index >= dbg.breakpoints().size()) {
            fmt::print("Index not valid.\n");
            return false;
        }
        dbg.delbreak(index.value());
        fmt::print("Breakpoint #{} deleted.\n", index.value());
        return false;
    }

    case Command::LIST_BREAK: {
        const auto &breaks = dbg.breakpoints();
        if (breaks.size() == 0)
            fmt::print("No breakpoints set.\n");
        else
            for (const auto &x : breaks)
                fmt::print("#: {:04X}-{:04X}, mode: {}\n", x.start, x.end, x.mode);
        return false;
    }

    case Command::BACKTRACE: {
        const auto &buf = dbg.backtrace();
        if (buf.size() == 0)
            fmt::print("Backtrace is empty.\n");
        else
            for (const auto &x : dbg.backtrace())
                fmt::print("${:02X}: {}\n", x.pc, x.str);
        return false;
    }

    case Command::DISASSEMBLE: {
        auto id      = Util::strconv<uint8>(args[0], 16);
        auto operand = args.size() == 1 ? 0 : Util::strconv<uint16>(args[1], 16);
        if (!id || !operand) {
            fmt::print("Invalid value found while parsing command arguments.\n");
            return false;
        }
        auto [str, _] = disassemble(id.value(), operand.value() >> 8, operand.value());
        fmt::print("{}\n", str);
        return false;
    }

    case Command::NEXT:
        dbg.next(instr);
        return true;

    case Command::STEP:
        dbg.step(instr);
        return true;

    case Command::STATUS:
        if (args.size() > 0 && args[0] == "ppu")
            fmt::print("{}", dbg.status(Debugger::StatusDev::PPU));
        else
            fmt::print("{}", dbg.status(Debugger::StatusDev::CPU));
        return false;

    case Command::READ_ADDR: {
        auto addr = Util::strconv<uint16>(args[0], 16);
        if (!addr) {
            fmt::print("Invalid address: {}.\n", args[0]);
            return false;
        }
        fmt::print("{:02X}\n", dbg.read(addr.value()));
        return false;
    }

    case Command::WRITE_ADDR: {
        auto addr   = Util::strconv<uint16>(args[0], 16);
        auto newval = Util::strconv<uint8 >(args[1], 16);
        if (!addr || !newval) {
            fmt::print("Invalid value found while parsing command arguments.\n");
            return false;
        }
        if (addr.value() >= CARTRIDGE_START)
            fmt::print("Warning: writes to ROM have no effects\n");
        dbg.write(addr.value(), newval.value());
        return false;
    }

    case Command::BLOCK: {
        auto start = Util::strconv<uint16>(args[0], 16);
        auto end   = Util::strconv<uint16>(args[1], 16);
        if (!start || !end || end <= start) {
            fmt::print("Invalid range.\n");
            return false;
        }
        print_block([&](uint16 addr) { return dbg.read(addr); }, start.value(), end.value());
        return false;
    }

    case Command::DISBLOCK: {
        auto start = Util::strconv<uint16>(args[0], 16);
        auto end   = Util::strconv<uint16>(args[1], 16);
        if (!start || !end || end <= start) {
            fmt::print("Invalid range.\n");
            return false;
        }
        disassemble_block(start.value(), end.value(),
                [&](uint16 addr) { return dbg.read(addr); },
                [] (uint16 addr, std::string &&s) { fmt::print("${:04X}: {}\n", addr, s); });
        return false;
    }

    case Command::RESET:
        return false;

    case Command::QUIT:
        dbg.set_quit(true);
        return true;

    case Command::INVALID:
        fmt::print("Invalid command. Try 'help'.\n");
        return false;

    default:
        return false;
    }
}

void clirepl(Debugger &dbg, Debugger::Event &ev)
{
    if (ev.type == Debugger::Event::Type::BREAK)
        fmt::print("Breakpoint #{} reached.\n", ev.index);
    fmt::print("${:04X}: [{:02X}] {}\n", ev.instr.pc, ev.instr.id, ev.instr.str);
    bool exit = false;
    do {
        fmt::print("> ");
        auto [cmd, args] = parse();
        auto &cmdinfo = cmd_infotab.find(cmd)->second;
        if (cmd != Command::INVALID && args.size() < cmdinfo.minargs) {
            fmt::print("Not enough arguments for command {}. Try 'help'.\n", cmdinfo.name);
            continue;
        }
        exit = exec_command(dbg, cmd, args, ev.instr);
    } while (!exit);
}

} // namespace Core


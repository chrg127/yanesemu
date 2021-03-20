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
    uint16 pc     = emu->cpu.pc.reg;
    Opcode opcode = emu->cpu.disassemble();
    if (mode == 0b001 && is_jump(opcode.code)) {
        tracebuf.push_back(std::make_pair(pc, opcode));
    } else if (mode == 0b001 && opcode.code == 0x60) {
        if (tracebuf.back().second.code == 0x20)
            tracebuf.pop_back();
    }
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
            ev.pc = pc;
            ev.opcode = opcode;
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

std::string Debugger::regs() const
{
    return emu->cpu.status();
}

void Debugger::reset()
{
    // TODO: there are a lot more things to reset
    nextstop = 0;
    emu->reset();
}

uint8 Debugger::read(uint16 addr)
{
    /* The PPU has a bunch of regs with side-effects on read.
     * If we are reading those, read them directly instead of going
     * through the bus. */
    if (addr == 0x2002)
        return emu->ppu.io.vblank << 7 | emu->ppu.io.sp_zero_hit << 6 | emu->ppu.io.sp_overflow << 5;
    else if (addr == 0x2007) {
        // always return the read buffer, without ever updating it
        return emu->ppu.vram.addr < 0x3F00 ? emu->ppu.io.data_buf
                                           : emu->vrambus.read(emu->ppu.vram.addr);
    }
    return emu->rambus.read(addr);
}

void Debugger::write(uint16 addr, uint8 value)
{
    emu->rambus.write(addr, value);
}



enum Command {
    HELP,
    BREAK,
    // WATCH,
    LIST_BREAK,
    DELBREAK,
    // LIST_WATCH,
    BACKTRACE,
    CONTINUE,
    DISASSEMBLE,
    NEXT,
    STEP,
    // STEP_OUT,
    // STATUS,
    REGS,
    READ_ADDR,
    WRITE_ADDR,
    RESET,
    QUIT,
    INVALID,
};

//  name                abbrev  enum                    min args    description
#define O(X) \
    X("help",           "h",    Command::HELP,          0,          "prints this help text") \
    X("break",          "b",    Command::BREAK,         3,          "set a breakpoint") \
    X("listbreaks",     "lb",   Command::LIST_BREAK,    0,          "list breakpoints") \
    X("deletebreak",    "dlb",  Command::DELBREAK,      1,          "delete a breakpoint") \
    X("backtrace",      "bt",   Command::BACKTRACE,     0,          "prints the backtrace") \
    X("continue",       "c",    Command::CONTINUE,      0,          "start/continue execution") \
    X("disassemble",    "dis",  Command::DISASSEMBLE,   1,          "disassemble the current instruction") \
    X("next",           "n",    Command::NEXT,          0,          "run next instruction") \
    X("step",           "s",    Command::STEP,          0,          "step next instruction") \
    X("regs",           "r",    Command::REGS,          0,          "print registers") \
    X("read",           "rd",   Command::READ_ADDR,     1,          "read address") \
    X("write",          "wr",   Command::WRITE_ADDR,    2,          "write address") \
    X("reset",          "res",  Command::RESET,         0,          "reset the machine") \
    X("quit",           "q",    Command::QUIT,          0,          "quit the emulator") \

#define X(name, abbrev, enumname, minargs, desc) { name, enumname }, { abbrev, enumname },
static const std::unordered_map<std::string, Command> strtocmd = {
    O(X)
};
#undef X

struct CommandInfo {
    std::string name;
    std::string desc;
    std::string abbrev;
    unsigned minargs;
};

#define X(name, abbrev, enumname, minargs, desc) { enumname, CommandInfo{ name, desc, abbrev, minargs } },
static std::unordered_map<Command, CommandInfo> cmd_to_info = {
    O(X)
};
#undef X

#define X(name, abbrev, enumname, minargs, desc) name ", " abbrev ": " desc "\n"
static const std::string helpstr = ""
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
    auto it = strtocmd.find(cmd);
    if (it == strtocmd.end())
        return std::make_pair(Command::INVALID, CmdArgs{});
    return std::make_pair(it->second, Util::strsplit(args, ' '));
}

/* execute a command and return whether to quit the repl. */
static bool exec_command(const Command cmd, const CmdArgs &args, Debugger &dbg, Debugger::Event &ev)
{
    switch (cmd) {

    case Command::HELP:
        fmt::print("{}", helpstr);
        return false;

    case Command::BREAK:
        return false;

    case Command::DELBREAK:
        return false;

    case Command::LIST_BREAK: {
        const auto &breaks = dbg.breakpoints();
        if (breaks.size() == 0)
            fmt::print("No breakpoints set.\n");
        else {
            for (const auto &x : breaks) {
                fmt::print("#: {:04X}-{:04X}, mode: \n", x.start, x.end,
                        x.mode == 0b100 ? "read"
                      : x.mode == 0b010 ? "write"
                      :                   "exec");

            }
        }
        return false;
    }

    case Command::BACKTRACE: {
        const auto &buf = dbg.tracebuffer();
        if (buf.size() == 0)
            fmt::print("Backtrace is empty.\n");
        else
            for (const auto &x : dbg.tracebuffer())
                fmt::print("${:02X}: {}\n", x.first, x.second.info.str);
        return false;
    }

    case Command::CONTINUE:
        fmt::print("Continuing.\n");
        dbg.continue_exec();
        return true;

    case Command::DISASSEMBLE: {
        auto code = Util::strtohex(args[0], 2);
        if (!code) {
            fmt::print("Invalid value: {}\n", args[0]);
            return false;
        }
        if (args.size() == 1) {
            fmt::print("{}\n", disassemble(code.value(), 0, 0).str);
            return false;
        }
        auto operand = Util::strtohex(args[1], 4);
        if (!operand) {
            fmt::print("Invalid value found while parsing command arguments.\n");
            return false;
        }
        fmt::print("{}\n", disassemble(code.value(), operand.value() >> 8, operand.value()).str);
        return false;
    }

    case Command::NEXT:
        dbg.next(ev.opcode);
        return true;

    case Command::STEP:
        dbg.step(ev.opcode);
        return true;

    case Command::REGS:
        fmt::print("{}\n", dbg.regs());
        return false;

    case Command::READ_ADDR: {
        auto addr = Util::strtohex(args[0], 4);
        if (!addr) {
            fmt::print("Invalid value: {}\n", args[0]);
            return false;
        }
        fmt::print("{:02X}\n", dbg.read(addr.value()));
        return false;
    }

    case Command::WRITE_ADDR: {
        auto addr   = Util::strtohex(args[0], 4);
        auto newval = Util::strtohex(args[1], 2);
        if (!addr || !newval) {
            fmt::print("Invalid value found while parsing command arguments.\n");
            return false;
        }
        if (addr.value() >= CARTRIDGE_START)
            fmt::print("Warning: writes to ROM have no effects\n");
        dbg.write(addr.value(), newval.value());
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
    fmt::print("${:04X}: [{:02X}] {}\n", ev.pc, ev.opcode.code, ev.opcode.info.str);
    bool exit = false;
    do {
        fmt::print("> ");
        auto p = parse();
        Command cmd   = p.first;
        auto &args    = p.second;
        auto &cmdinfo = cmd_to_info.find(cmd)->second;
        if (args.size() < cmdinfo.minargs) {
            fmt::print("Not enough arguments for command {}. Try 'help'.\n", cmdinfo.name);
            continue;
        }
        exit = exec_command(cmd, args, dbg, ev);
    } while (!exit);
}

} // namespace Core


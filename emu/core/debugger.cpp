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
    if (addr >= CARTRIDGE_START)
        fmt::print("warning: writes to ROM have no effects\n");
    emu->rambus.write(addr, value);
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
    // STEP_OUT,
    // STATUS,
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

using CmdArgs = std::vector<std::string>;

static std::pair<Command, CmdArgs> parse()
{
    Util::File input{stdin};
    std::string cmd = "", args = "";

    if (!input.getword(cmd) || !input.getline(args))
        return std::make_pair(Command::QUIT, CmdArgs{});
    if (cmd == "")
        return std::make_pair(Command::INVALID, CmdArgs{});
    auto it = strtocmd.find(cmd);
    if (it == strtocmd.end())
        return std::make_pair(Command::INVALID, CmdArgs{});
    return std::make_pair(it->second, Util::strsplit(args, ' '));
}

static void read_command(Debugger &dbg, const CmdArgs &args)
{
    if (args.size() < 1) {
        fmt::print("read: not enough arguments. see 'help' for more information.\n");
        return;
    }
    const std::string &as = args[0];
    auto addr = Util::strtohex(as, 4);
    if (!addr) {
        fmt::print("{}: invalid address\n", as);
        return;
    }
    fmt::print("{:02X}\n", dbg.read(addr.value()));
}

static void write_command(Debugger &dbg, const CmdArgs &args)
{
    if (args.size() < 2) {
        fmt::print("write: not enough arguments. see 'help' for more information.\n");
        return;
    }
    const std::string &as = args[0];
    const std::string &vs = args[1];
    auto addr = Util::strtohex(as, 4);
    if (!addr) {
        fmt::print("{}: invalid address.\n", as);
        return;
    }
    auto newval = Util::strtohex(vs, 2);
    if (!newval) {
        fmt::print("{}: invalid value.\n", vs);
        return;
    }
    dbg.write(addr.value(), newval.value());
}


void clirepl(Debugger &dbg, Debugger::Event &ev)
{
    fmt::print("${:04X}: [{:02X}] {}\n", ev.pc, ev.opcode.code, ev.opcode.info.str);
    bool exit = false;
    do {
        fmt::print("> ");
        auto p = parse();
        Command cmd = p.first;
        auto args   = std::move(p.second);

        switch (cmd) {
        case Command::HELP:
            for (const auto &cmd : cmdinfo)
                fmt::print("{}, {}: {}\n", cmd.name, cmd.abbrev, cmd.desc);
            break;
        case Command::BREAK:
            break;
        case Command::LIST_BREAK:
            break;
        case Command::BACKTRACE:
            for (const auto &x : dbg.tracebuffer())
                fmt::print("{:02X}: {}\n", x.first, x.second.info.str);
            break;
        case Command::CONTINUE:
            fmt::print("continuing.\n");
            dbg.continue_exec();
            exit = true;
            break;
        case Command::DISASSEMBLE: {
            if (args.size() < 1) {
                fmt::print("disasemble: not enough arguments\n");
                break;
            }
            auto code = Util::strtohex(args[0], 2);
            if (!code) {
                fmt::print("disassemble: {}: invalid value\n", code.value());
                break;
            }
            if (args.size() == 1) {
                fmt::print("{}\n", disassemble(code.value(), 0, 0).str);
                break;
            }
            auto operand = Util::strtohex(args[1], 4);
            if (!operand) {
                fmt::print("disassemble: invalid value found while parsing args\n");
                break;
            }
            fmt::print("{}\n", disassemble(code.value(), operand.value() >> 8, operand.value()).str);
            break;
        }
        case Command::NEXT:
            dbg.next(ev.opcode);
            exit = true;
            break;
        case Command::STEP:
            dbg.step(ev.opcode);
            exit = true;
            break;
        case Command::REGS:
            fmt::print("{}\n", dbg.regs());
            break;
        case Command::READ_ADDR:
            read_command(dbg, args);
            break;
        case Command::WRITE_ADDR:
            write_command(dbg, args);
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


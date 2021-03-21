#include <emu/core/debugger.hpp>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <emu/core/emulator.hpp>
#include <emu/util/stringops.hpp>
#include <emu/util/file.hpp>

namespace Core {

void Debugger::fetch_callback(uint16 addr, char mode)
{
    if (quit)
        return;

    uint16 pc     = emu->cpu.pc.reg;
    Opcode opcode = emu->cpu.disassemble();

    // trace all jumps by default
    if (mode == 'x') {
        switch (opcode.code) {
        case 0x20: case 0x4C: case 0x6C:
            tracebuf.push_back(std::make_pair(pc, opcode));
            break;
        case 0x60:
            if (tracebuf.back().second.code == 0x20)
                tracebuf.pop_back();
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
            .pc     = pc,
            .opcode = opcode,
            .index  = it - breakvec.begin(),
        };
        callback(*this, ev);
    }

    // handle next/step commands
    if (nextstop) {
        /* If the user has issued a next/step command, but an interrupt
         * happened, modify the next stop address so that we get inside the interrupt handler.
         * The interrupt vector is read 2 times, so make sure we ignore
         * the second read (can be done by checking addr) */
        if (mode == 'r' && addr >= 0xFFFA && (addr & 1) == 0) {
            nextstop = emu->rambus.read(addr+1) << 8 | emu->rambus.read(addr);
        } else if (mode == 'x' && addr == nextstop.value()) {
            nextstop.reset();
            Event ev = {
                .type   = Event::Type::FETCH,
                .pc     = pc,
                .opcode = opcode,
                .index  = 0,
            };
            callback(*this, ev);
        }
    }
}

void Debugger::step(const Opcode &op)
{
    nextstop = emu->cpu.nextaddr(op);
}

void Debugger::next(const Opcode &op)
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
    switch (addr) {
    case 0x2002: return emu->ppu.io.vblank << 7 | emu->ppu.io.sp_zero_hit << 6 | emu->ppu.io.sp_overflow << 5;
    // always return the read buffer, without ever updating it
    case 0x2007: return emu->ppu.vram.addr < 0x3F00 ? emu->ppu.io.data_buf
                                                    : emu->vrambus.read(emu->ppu.vram.addr);
    default: return emu->rambus.read(addr);
    }
}

void Debugger::write(uint16 addr, uint8 value)
{
    emu->rambus.write(addr, value);
}

std::vector<uint8> Debugger::readblock(uint16 start, uint16 end)
{
    std::vector<uint8> block;
    for (uint16 addr = start; addr != end; addr++) {
        block.push_back(read(addr));
    }
    return block;
}


struct CommandInfo {
    std::string name;
    std::string desc;
    std::string abbrev;
    unsigned minargs;
};

//  name                abbrev  enum                    min args    description
#define O(X) \
    X("help",           "h",    HELP,          0,          "prints this help text") \
    X("break",          "b",    BREAK,         2,          "set a breakpoint") \
    X("listbreaks",     "lb",   LIST_BREAK,    0,          "list breakpoints") \
    X("deletebreak",    "dlb",  DELBREAK,      1,          "delete a breakpoint") \
    X("backtrace",      "bt",   BACKTRACE,     0,          "prints the backtrace") \
    X("continue",       "c",    CONTINUE,      0,          "start/continue execution") \
    X("disassemble",    "dis",  DISASSEMBLE,   1,          "disassemble the current instruction") \
    X("next",           "n",    NEXT,          0,          "run next instruction") \
    X("step",           "s",    STEP,          0,          "step next instruction") \
    X("regs",           "r",    REGS,          0,          "print registers") \
    X("read",           "rd",   READ_ADDR,     1,          "read address") \
    X("write",          "wr",   WRITE_ADDR,    2,          "write address") \
    X("block",          "bl",   BLOCK,         2,          "read block") \
    X("reset",          "res",  RESET,         0,          "reset the machine") \
    X("quit",           "q",    QUIT,          0,          "quit the emulator") \

#define X(name, abbrev, enumname, minargs, desc) enumname,
enum Command {
    O(X)
    INVALID,
};
#undef X

#define X(name, abbrev, enumname, minargs, desc) { name, enumname }, { abbrev, enumname },
static const std::unordered_map<std::string, Command> strtocmd = {
    O(X)
};
#undef X

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

static std::optional<uint16> addrconv(const std::string &str)
{
    return Util::strconv<uint16>(str, 4, 16);
}

static void print_block(std::vector<uint8> &&block, uint16 start, uint16 end)
{
    for (unsigned i = 0; i < block.size(); ) {
        fmt::print("{:04X}: ", i + start);
        for (unsigned j = 0; j < 16 && i + start < end; j++) {
            fmt::print("{:02X} ", block[i]);
            i++;
        }
        fmt::print("\n");
    }
}

/* execute a command and return whether to quit the repl. */
static bool exec_command(Debugger &dbg, const Command cmd, const CmdArgs &args, const Opcode &opcode)
{
    switch (cmd) {

    case Command::HELP:
        fmt::print("{}", helpstr);
        return false;

    case Command::BREAK: {
        char mode = args[0][0];
        if (args[0].size() != 1 || (mode != 'r' && mode != 'w' && mode != 'x')) {
            fmt::print("Invalid mode for breakpoint. Try 'help'.\n");
            return false;
        }
        auto start = addrconv(args[1]);
        auto end = args.size() > 2 ? addrconv(args[2]) : start;
        if (!start || !end || end <= start) {
            fmt::print("Invalid range.\n");
            return false;
        }
        unsigned i = dbg.setbreak({ .start = start.value(), .end = end.value(), .mode = mode });
        fmt::print("Set breakpoint {} to {:04X}-{:04X}.\n", i, start.value(), end.value());
        return false;
    }

    case Command::DELBREAK: {
        auto index = addrconv(args[0]);
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
        auto code = Util::strconv<uint8>(args[0], 2, 16);
        if (!code) {
            fmt::print("Invalid value: {}\n", args[0]);
            return false;
        }
        auto operand = args.size() == 1 ? 0 : addrconv(args[1]);
        if (!operand) {
            fmt::print("Invalid value found while parsing command arguments.\n");
            return false;
        }
        fmt::print("{}\n", disassemble(code.value(), operand.value() >> 8, operand.value()).str);
        return false;
    }

    case Command::NEXT:
        dbg.next(opcode);
        return true;

    case Command::STEP:
        dbg.step(opcode);
        return true;

    case Command::REGS:
        fmt::print("{}\n", dbg.regs());
        return false;

    case Command::READ_ADDR: {
        auto addr = addrconv(args[0]);
        if (!addr) {
            fmt::print("Invalid address: {}.\n", args[0]);
            return false;
        }
        fmt::print("{:02X}\n", dbg.read(addr.value()));
        return false;
    }

    case Command::WRITE_ADDR: {
        auto addr   = addrconv(args[0]);
        auto newval = Util::strconv<uint8>(args[1], 2, 16);
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
        auto start = addrconv(args[0]);
        auto end   = addrconv(args[1]);
        if (!start || !end || end <= start) {
            fmt::print("Invalid range.\n");
            return false;
        }
        print_block(dbg.readblock(start.value(), end.value()), start.value(), end.value());
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
    fmt::print("${:04X}: [{:02X}] {}\n", ev.pc, ev.opcode.code, ev.opcode.info.str);
    bool exit = false;
    do {
        fmt::print("> ");
        auto [cmd, args] = parse();
        auto &cmdinfo = cmd_to_info.find(cmd)->second;
        if (cmd != Command::INVALID && args.size() < cmdinfo.minargs) {
            fmt::print("Not enough arguments for command {}. Try 'help'.\n", cmdinfo.name);
            continue;
        }
        exit = exec_command(dbg, cmd, args, ev.opcode);
    } while (!exit);
}

} // namespace Core


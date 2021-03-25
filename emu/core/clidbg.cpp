#include <emu/core/clidbg.hpp>

#include <unordered_map>
#include <fmt/core.h>
#include <emu/util/unsigned.hpp>
#include <emu/core/instrinfo.hpp>
#include <emu/core/cpu.hpp> // CPU::Status
#include <emu/core/ppu.hpp> // PPU::Status
#include <emu/core/debugger.hpp>
#include <emu/util/stringops.hpp>
#include <emu/util/file.hpp>

namespace Core {

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
    X("trace",          "tr",   TRACE,         1,          "") \
    X("stoptrace",       "str",  STOP_TRACE,   0,          "") \
    X("reset",          "res",  RESET,         0,          "reset the machine") \
    X("quit",           "q",    QUIT,          0,          "quit the emulator") \


#define X(name, abbrev, enumname, minargs, desc) { name, enumname }, { abbrev, enumname },
static const std::unordered_map<std::string, Command> cmd_strtab = {
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
static std::unordered_map<Command, CommandInfo> cmd_infotab = {
    O(X)
    { Command::INVALID, CommandInfo{} },
};
#undef X

static const std::string helpstr = R"(
help, h:            prints this help text
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

static void print_block(uint16 start, uint16 end, auto &&readvalue)
{
    for (uint16 i = 0; i < (end - start); ) {
        fmt::print("{:04X}: ", i + start);
        for (uint16 j = 0; j < 16 && i + start < end; j++) {
            uint8 val = readvalue(i + start);
            fmt::print("{:02X} ", val);
            i++;
        }
        fmt::print("\n");
    }
}

static void print_cpu_status(CPU::Status &&st)
{
    fmt::print("PC: ${:02X} A: ${:02X} X: ${:02X} Y: ${:02X} S: ${:02X}\n"
               "Flags: [{}]\n"
               "Cycles: {}\n",
        st.regs.pc.full, st.regs.acc, st.regs.x, st.regs.y, st.regs.sp,
        format_flags(st.regs.flags), st.regs.cycles
    );
}

static void print_ppu_status(PPU::Status &&st)
{
    fmt::print("Line: {} Cycle: {}\nVRAM Address: {:04X} TMP Address: {:04X}\n",
               st.line, st.cycle, st.vram.addr, st.vram.tmp);
}

static void print_curr_instr(CPU::Status &st)
{
    fmt::print("${:04X}: {}\n", st.regs.pc.full, format_instr(
            st.instr.id, st.instr.op.low, st.instr.op.high,
            st.regs.pc.full, st.regs.flags
        )
    );
}


/* execute a command and return whether to quit the repl. */
static bool exec_command(Debugger &dbg, const Command cmd, const CmdArgs &args)
{
    switch (cmd) {

    case Command::HELP:
        fmt::print("{}", helpstr);
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
        fmt::print("Set breakpoint #{} to {:04X}-{:04X}.\n", i, start.value(), end.value());
        return false;
    }

    case Command::DELBREAK: {
        auto index = Util::strconv<uint16>(args[0], 16);
        if (!index || index >= dbg.breakpoints().size())
            fmt::print("Index not valid.\n");
        else {
            dbg.delbreak(index.value());
            fmt::print("Breakpoint #{} deleted.\n", index.value());
        }
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
                fmt::print("${:04X}: {}\n", x.regs.pc.full, disassemble(x.instr.id, x.instr.op.low, x.instr.op.high));
        return false;
    }

    case Command::DISASSEMBLE: {
        auto id     = Util::strconv<uint8>(args[0], 16);
        auto oplow  = args.size() >= 2 ? Util::strconv<uint16>(args[1], 16) : 0;
        auto ophigh = args.size() >= 3 ? Util::strconv<uint16>(args[2], 16) : 0;
        if (!id || !oplow || !ophigh)
            fmt::print("Invalid value found while parsing command arguments.\n");
        else
            fmt::print("{}\n", disassemble(id.value(), oplow.value(), ophigh.value()));
        return false;
    }

    case Command::NEXT:
        dbg.next();
        return true;

    case Command::STEP:
        dbg.step();
        return true;

    case Command::STATUS:
        if (args.size() > 0 && args[0] == "ppu")
            print_ppu_status(dbg.ppu_status());
        else
            print_cpu_status(dbg.cpu_status());
        return false;

    case Command::READ_ADDR: {
        auto addr = Util::strconv<uint16>(args[0], 16);
        if (!addr)
            fmt::print("Invalid address: {}.\n", args[0]);
        else
            fmt::print("{:02X}\n", dbg.read(addr.value()));
        return false;
    }

    case Command::WRITE_ADDR: {
        auto addr   = Util::strconv<uint16>(args[0], 16);
        auto newval = Util::strconv<uint8 >(args[1], 16);
        if (!addr || !newval)
            fmt::print("Invalid value found while parsing command arguments.\n");
        else {
            if (addr.value() >= CARTRIDGE_START)
                fmt::print("Warning: writes to ROM have no effects\n");
            dbg.write(addr.value(), newval.value());
        }
        return false;
    }

    case Command::BLOCK: {
        auto start = Util::strconv<uint16>(args[0], 16);
        auto end   = Util::strconv<uint16>(args[1], 16);
        if (!start || !end || end <= start)
            fmt::print("Invalid range.\n");
        else {
            print_block(start.value(), end.value(),
                        [&](uint16 addr) { return dbg.read(addr); });
        }
        return false;
    }

    case Command::DISBLOCK: {
        auto start = Util::strconv<uint16>(args[0], 16);
        auto end   = Util::strconv<uint16>(args[1], 16);
        if (!start || !end || end <= start)
            fmt::print("Invalid range.\n");
        else {
            disassemble_block(start.value(), end.value(),
                [&](uint16 addr) { return dbg.read(addr); },
                [] (uint16 addr, std::string &&s) { fmt::print("${:04X}: {}\n", addr, s); });
        }
        return false;
    }

    case Command::TRACE: {
        Util::File f(args[0], Util::File::Mode::WRITE);
        if (!f)
            fmt::print("{}\n", f.error_str());
        else
            dbg.start_tracing(std::move(f));
        return false;
    }

    case Command::STOP_TRACE:
        dbg.stop_tracing();
        return false;

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

static std::pair<Command, CmdArgs> parse()
{
    Util::File input{stdin};
    std::string cmd, args;

    if (!input.getword(cmd) || !input.getline(args))
        return std::make_pair(Command::QUIT, CmdArgs{});
    if (cmd.empty())
        return std::make_pair(Command::LAST, CmdArgs{});
    auto it = cmd_strtab.find(cmd);
    if (it == cmd_strtab.end())
        return std::make_pair(Command::INVALID, CmdArgs{});
    return std::make_pair(it->second, Util::strsplit(args, ' '));
}

void CliDebugger::repl(Debugger &dbg, Debugger::Event &&ev)
{
    if (ev.type == Debugger::Event::Type::BREAK)
        fmt::print("Breakpoint #{} reached.\n", ev.index);
    print_curr_instr(ev.cpu_st);

    bool exit = false;
    do {
        fmt::print("> ");
        auto [input_cmd, input_args] = parse();
        if (input_cmd != Command::LAST) {
            cmd     = input_cmd;
            cmdargs = std::move(input_args);
        }
        if (const auto &info = cmd_infotab.find(cmd)->second;
            cmdargs.size() < info.minargs) {
            fmt::print("Not enough arguments for command {}. Try 'help'.\n", info.name);
            continue;
        }
        exit = exec_command(dbg, cmd, cmdargs);
    } while (!exit);
}

} // namespace Core

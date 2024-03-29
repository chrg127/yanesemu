#include "clidebugger.hpp"

#include <fmt/core.h>
#include <emu/program.hpp>
#include <emu/core/const.hpp>
#include <emu/util/callcommand.hpp>
#include <emu/util/string.hpp>
#include <emu/util/bits.hpp>

using util::ParseError;
using util::Command;

#define CONVERT_FUNC(type, how, msg)                                    \
template <> type util::try_convert_impl<type>(std::string_view str) {   \
    if (auto o = how; o) return o.value();                              \
    throw ParseError(fmt::format(msg, str));                            \
}                                                                       \

template <> std::string_view util::try_convert_impl<std::string_view>(std::string_view str) { return str; }
CONVERT_FUNC(u16,                        str::to_num<u16>(str, 16),          "Invalid address: {}.");
CONVERT_FUNC(u8,                         str::to_num<u8>(str, 16),           "Invalid value: {}.");
CONVERT_FUNC(int,                        str::to_num(str),                   "Not a number: {}.");
CONVERT_FUNC(debugger::MemorySource,     debugger::string_to_memsource(str), "Invalid memory source: {}.");
CONVERT_FUNC(debugger::Component,        debugger::string_to_component(str), "Invalid component: {}.");
CONVERT_FUNC(debugger::CPUDebugger::Reg, debugger::string_to_cpu_reg(str),   "Invalid register: {}.");
CONVERT_FUNC(debugger::PPUDebugger::Reg, debugger::string_to_ppu_reg(str),   "Invalid register: {}.");
CONVERT_FUNC(input::Button,              input::string_to_button(str),       "Invalid button: {}.");

namespace debugger {

static void check_addr_ranges(u16 start, u16 end, MemorySource source)
{
    if (end < start)
        throw ParseError(fmt::format("Invalid range: {:04X}-{:04X}.", start, end));
    if (source == MemorySource::VRAM && (start > 0x4000 || end > 0x4000))
        throw ParseError(fmt::format("Invalid range for source VRAM."));
    if (source == MemorySource::OAM && (start > 0xFF || end > 0xFF))
        throw ParseError(fmt::format("Invalid range for source OAM."));
}

void CliDebugger::help()
{
    fmt::print("help, h:            print this help text\n"
               "continue, c:        start or continue execution\n"
               "runframe, nmi:      run for an entire frame, stop at nmi handler\n"
               "next, n:            run next instruction (will skip subroutines)\n"
               "step, s:            step next instruction\n"
               "break, b:           set a breakpoint at address or address range\n"
               "listbreaks, lb      list breakpoints\n"
               "deletebreak, delb   delete a breakpoint\n"
               "status, st          print current status of a component. default: cpu.\n"
               "read, rd            read value from address. source can be ram or vram.\n"
               "write, wr           write a value to an address\n"
               "block, bl           read a block of memory\n"
               "disassemble, dis:   disassemble 3 bytes\n"
               "disblock, db:       disassemble a block of memory\n"
               "writecpureg, wrcpu: writes a value to a CPU register\n"
               "writeppureg, wrppu: writes a value to a PPU register\n"
               "hold, hb:           select a button to hold automatically\n"
               "unhold, uhb         stop holding button automatically\n"
               "trace, t:           trace and log instructions to a file\n"
               "stoptrace, str:     stop tracing instructions\n"
               "reset, r:           reset emulator\n",
               "quit, q:            quit the debugger\n");
}

void CliDebugger::breakpoint(u16 start, u16 end)
{
    if (end < start)
        throw ParseError(fmt::format("Invalid range."));
    auto i = break_list.add({ .start = start, .end = end });
    fmt::print("Set breakpoint #{} to {:04X}-{:04X}.\n", i, start, end);
}

void CliDebugger::delete_breakpoint(int index)
{
    if (index >= int(break_list.size()))
        throw ParseError(fmt::format("No breakpoint number {}.", index));
    break_list.erase(index);
    fmt::print("Breakpoint #{} deleted.\n", index);
}

void CliDebugger::list_breakpoints()
{
    for (std::size_t i = 0; i < break_list.size(); i++)
        if (!break_list[i].erased)
            fmt::print("#{}: {:04X}-{:04X}\n", i, break_list[i].start, break_list[i].end);
}

void CliDebugger::status(Component component)
{
    switch (component) {
    case Component::CPU: print_cpu_status(); break;
    case Component::PPU: print_ppu_status(); break;
    }
}

void CliDebugger::read_block(u16 start, u16 end, MemorySource source)
{
    check_addr_ranges(start, end, source);
    auto readfn = read_from(source);
    for (int i = 0; i <= (end - start);) {
        fmt::print("${:04X}: ", start + i);
        for (int j = 0; j < 16 && i + start <= end; i++, j++)
            fmt::print("{:02X} ", readfn(start + i));
        fmt::print("\n");
    }
}

void CliDebugger::write_addr(u16 addr, u8 value, MemorySource source)
{
    if (source == MemorySource::RAM && addr >= core::CARTRIDGE_START)
        fmt::print("Warning: writes to ROM have no effects\n");
    write_block(addr, addr, value, source);
}

void CliDebugger::write_block(u16 start, u16 end, u8 data, MemorySource source)
{
    check_addr_ranges(start, end, source);
    auto writefn = write_to(source);
    for (u16 curr = start; curr <= end; curr++)
        writefn(curr, data);
}

void CliDebugger::disassemble(u8 id, u8 low, u8 high)
{
    fmt::print("{}\n", debugger::disassemble(id, low, high).first);
}

void CliDebugger::disassemble_block(u16 start, u16 end)
{
    check_addr_ranges(start, end, MemorySource::RAM);
    debugger::disassemble_block(start, end,
        [&](u16 addr)                       { return read_ram(addr); },
        [ ](u16 addr, std::string_view str) { fmt::print("${:04X}: {}\n", addr, str); });
}

void CliDebugger::trace(std::string_view filename)
{
    if (!tracer.start(filename))
        std::perror("error");
}

static std::string parse_error_message(int which, std::string_view name, int num_params)
{
    switch (which) {
    case 0: return fmt::format("Invalid command: {}.", name);
    case 1: return fmt::format("Wrong number of parameters for command {} (got {})", name, num_params);
    default: return "";
    }
}

bool CliDebugger::repl()
{
    bool quit = false;
    auto input = io::File::assoc(stdin);
    std::string line;

    fmt::print(">>> ");
    if (!input.get_line(line))
        return true;

    try {
        if (!line.empty())
            last_args = str::split(line, ' ');
        util::call_command(last_args, parse_error_message,
            Command{ "help",        "h",        &CliDebugger::help,                         this },
            Command{ "continue",    "c",        [&]() { run(Debugger::StepType::None); }         },
            Command{ "runframe",    "nmi",      [&]() { run(Debugger::StepType::Frame); }        },
            Command{ "next",        "n",        [&]() { run(Debugger::StepType::Next); }         },
            Command{ "step",        "s",        [&]() { run(Debugger::StepType::Step); }         },
            Command{ "break",       "b",        &CliDebugger::breakpoint_single,            this },
            Command{ "break",       "b",        &CliDebugger::breakpoint,                   this },
            Command{ "listbreaks",  "lb",       &CliDebugger::list_breakpoints,             this },
            Command{ "deletebreak", "delb",     &CliDebugger::delete_breakpoint,            this },
            Command{ "status",      "st",       &CliDebugger::status,                       this },
            Command{ "status",      "st",       [&]() { status(Component::CPU); }                },
            Command{ "read",        "rd",       &CliDebugger::read_addr,                    this },
            Command{ "read",        "rd",       &CliDebugger::read_addr_ram,                this },
            Command{ "write",       "wr",       &CliDebugger::write_addr,                   this },
            Command{ "write",       "wr",       &CliDebugger::write_addr_ram,               this },
            Command{ "block",       "bl",       &CliDebugger::read_block,                   this },
            Command{ "block",       "bl",       &CliDebugger::read_block_ram,               this },
            Command{ "disassemble", "dis",      &CliDebugger::disassemble,                  this },
            Command{ "disblock",    "db",       &CliDebugger::disassemble_block,            this },
            Command{ "writecpureg", "wrcpu",    &CliDebugger::write_cpu_reg,                this },
            Command{ "writeppureg", "wrppu",    &CliDebugger::write_ppu_reg,                this },
            Command{ "hold",        "hb",       &CliDebugger::hold_button,                  this },
            Command{ "unhold",      "uhb",      &CliDebugger::unhold_button,                this },
            Command{ "trace",       "t",        &CliDebugger::trace,                        this },
            Command{ "stoptrace",   "str",      [&]() { tracer.stop(); }                         },
            Command{ "reset",       "r",        [&]() { reset_system(); }                        },
            Command{ "quit",        "q",        [&]() { quit = true; }                           }
        );
    } catch (const ParseError &error) {
        fmt::print("{}\n", error.what());
    }

    return quit;
}

void CliDebugger::report_event(Debugger::Event ev)
{
    switch (ev.type) {
    case Debugger::Event::Type::Step:
        print_instr();
        break;
    case Debugger::Event::Type::Break:
        fmt::print("Breakpoint #{} reached.\n", ev.u.point_id);
        print_instr();
        break;
    case Debugger::Event::Type::InvalidInstruction:
        fmt::print("Found invalid instruction {:02X} at {:04X}.\n", ev.u.inv.id, ev.u.inv.addr);
        break;
    }
}

void CliDebugger::print_instr() const
{
    fmt::print("${:04X}: {}\n", cpu.reg(CPUDebugger::Reg::PC), cpu.curr_instr_to_string());
}

void CliDebugger::print_cpu_status() const
{
    fmt::print("PC: ${:02X} A: ${:02X} X: ${:02X} Y: ${:02X} S: ${:02X}\n"
               "Flags: [{}]\n"
               "Cycles: {}\n",
               cpu.reg(CPUDebugger::Reg::PC),
               cpu.reg(CPUDebugger::Reg::Acc),
               cpu.reg(CPUDebugger::Reg::X),
               cpu.reg(CPUDebugger::Reg::Y),
               cpu.reg(CPUDebugger::Reg::SP),
               cpu.flags_to_string(),
               cpu.cycles());
}

void CliDebugger::print_ppu_status() const
{
    auto onoff = [](auto val) { return val ? "ON" : "OFF"; };
    u8 ctrl = ppu.reg(PPUDebugger::Reg::Ctrl);
    u8 mask = ppu.reg(PPUDebugger::Reg::Mask);
    u8 status = ppu.reg(PPUDebugger::Reg::Status);
    auto [line, cycle] = ppu.pos();
    fmt::print("Line: {}; Cycle: {}\n", line, cycle);
    fmt::print("PPUCTRL ($2000): {:08b}:\n"
               "    Base nametable address: ${:04X}\n    VRAM address increment: {}\n    Sprite pattern table address: ${:04X}\n"
               "    Background pattern table address: ${:04X}\n    Sprite size: {}\n    Master/slave: {}\n    NMI enabled: {}\n",
               ctrl, ppu.nt_base_addr(), (ctrl & 4) == 0 ? 1 : 32, bits::getbit(ctrl, 4) * 0x1000,
               bits::getbit(ctrl, 5) * 0x1000, (ctrl & 32) ? "8x16" : "8x8", (ctrl & 64) ? "output color" : "read backdrop",
               (ctrl & 128) ? "ON" : "OFF");
    fmt::print("PPUMASK ($2001): {:08b}:\n"
               "    Greyscale: {}\n    BG left: {}\n    Sprites left: {}\n    BG: {}\n    Sprites: {}\n"
               "    Emphasize red: {}\n    Emphasize green: {}\n    Emphasize blue: {}\n",
               mask, onoff(mask & 1),  onoff(mask & 2),  onoff(mask & 4),  onoff(mask & 8),
               onoff(mask & 16), onoff(mask & 32), onoff(mask & 64), onoff(mask & 128));
    fmt::print("PPUSTATUS ($2002): {:08b}:\n"
               "    Sprite overflow: {}\n    Sprite 0 hit: {}\n    Vertical blank: {}\n",
               status, onoff(status & 32), onoff(status & 64), onoff(status & 128));
    fmt::print("OAMADDR ($2003): ${:02X}\n", ppu.reg(PPUDebugger::Reg::OAMAddr));
    fmt::print("VRAM address: {:04X}\n", ppu.vram_addr());
    fmt::print("TMP address: {:04X}\n", ppu.tmp_addr());
    fmt::print("Fine X: {:X}\n", ppu.fine_x());
}

} // namespace debugger

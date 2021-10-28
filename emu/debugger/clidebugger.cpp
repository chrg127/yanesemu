#include "clidebugger.hpp"

#include <fmt/core.h>
#include <emu/core/const.hpp>
#include <emu/util/repl.hpp>
#include <emu/util/string.hpp>
#include <emu/util/bits.hpp>

using util::ParseError;
using util::TryConvert;
using util::Command;

#define CONVERT_FUNC(type, how, msg)            \
template <> struct util::TryConvert<type> {     \
    static type convert(std::string_view str)   \
    {                                           \
        if (auto o = how; o) return o.value();  \
        throw ParseError(fmt::format(msg, str));\
    }                                           \
}

CONVERT_FUNC(u16,                    str::conv<u16>(str, 16),            "Invalid address: {}.");
CONVERT_FUNC(u8,                     str::conv<u8>(str, 16),             "Invalid value: {}.");
CONVERT_FUNC(int,                    str::conv(str),                     "Not a number: {}.");
CONVERT_FUNC(debugger::MemorySource, debugger::string_to_memsource(str), "Invalid memory source: {}.");
CONVERT_FUNC(debugger::Component,    debugger::string_to_component(str), "Invalid Component: {}.");

namespace debugger {

namespace {

void check_addr_ranges(u16 start, u16 end, MemorySource source)
{
    if (end < start)
        throw ParseError(fmt::format("Invalid range: {:04X}-{:04X}.", start, end));
    if (source == MemorySource::VRAM && (start > 0x4000 || end > 0x4000))
        throw ParseError(fmt::format("Invalid range for source VRAM."));
    if (source == MemorySource::OAM && (start > 0xFF || end > 0xFF))
        throw ParseError(fmt::format("Invalid range for source OAM."));
}

void read_block(Debugger &dbg, u16 start, u16 end, MemorySource source)
{
    check_addr_ranges(start, end, source);
    auto readfn = dbg.read_from(source);
    for (int i = 0; i <= (end - start);) {
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

} // namespace

void CliDebugger::help()
{
    fmt::print("help\n");
}

void CliDebugger::continue_exec() { dbg.advance(); }
void CliDebugger::runframe()      { dbg.advance_frame(); }
void CliDebugger::next()          { dbg.next(); }
void CliDebugger::step()          { dbg.step(); }

void CliDebugger::breakpoint_range(u16 start, u16 end)
{
    if (end < start)
        throw ParseError(fmt::format("Invalid range."));
    auto i = dbg.breakpoints().add({ .start = start, .end = end });
    fmt::print("Set breakpoint #{} to {:04X}-{:04X}.\n", i, start, end);
}
void CliDebugger::breakpoint(u16 addr) { breakpoint_range(addr, addr); }

void CliDebugger::delete_break(int index)
{
    if (index >= dbg.breakpoints().size())
        throw ParseError(fmt::format("No breakpoint number {}.", index));
    dbg.breakpoints().erase(index);
    fmt::print("Breakpoint #{} deleted.\n", index);
}

void CliDebugger::list_breakpoints()
{
    const auto &list = dbg.breakpoints();
    for (std::size_t i = 0; i < list.size(); i++) {
        if (list[i].erased)
            continue;
        fmt::print("#{}: {:04X}-{:04X}\n", i, list[i].start, list[i].end);
    }
}

void CliDebugger::status(Component component)
{
    switch (component) {
    case Component::CPU: print_cpu_status(); break;
    case Component::PPU: print_ppu_status(); break;
    }
}
void CliDebugger::status_cpu()                                   { status(Component::CPU); }

void CliDebugger::read_addr(u16 addr, MemorySource source)       { read_block(dbg, addr, addr, source); }
void CliDebugger::read_addr_ram(u16 addr)                        { read_addr(addr, MemorySource::RAM); }

void CliDebugger::write_addr(u16 addr, u8 value, MemorySource source)
{
    if (source == MemorySource::RAM && addr >= core::CARTRIDGE_START)
        fmt::print("Warning: writes to ROM have no effects\n");
    write_block(dbg, addr, addr, value, source);
}
void CliDebugger::write_addr_ram(u16 addr, u8 data)              { write_addr(addr, data, MemorySource::RAM); }

void CliDebugger::block(u16 start, u16 end, MemorySource source) { read_block(dbg, start, end, source); }
void CliDebugger::block_ram(u16 start, u16 end)                  { block(start, end, MemorySource::RAM); }

void CliDebugger::disassemble(u8 id, u8 low, u8 high)            { fmt::print("{}\n", debugger::disassemble(id, low, high)); }
void CliDebugger::disassemble_block(u16 start, u16 end)
{
    check_addr_ranges(start, end, MemorySource::RAM);
    debugger::disassemble_block(start, end,
        [&](u16 addr)                    { return dbg.read_ram(addr); },
        [ ](u16 addr, std::string &&str) { fmt::print("${:04X}: {}\n", addr, str); });
}

void CliDebugger::trace(std::string_view filename)
{
    if (!dbg.start_tracing(filename))
        std::perror("error");
}
void CliDebugger::stop_tracing() { dbg.stop_tracing(); }

void CliDebugger::reset() {}

static std::string parse_error_message(int which, std::string_view name, int num_params)
{
    // 0 = command not found
    // 1 = command found, wrong number of parameters
    if (which == 1)
        return fmt::format("Wrong number of parameters for command {} (got {})", name, num_params);
    return fmt::format("Invalid command: {}.", name);
}

bool CliDebugger::repl()
{
    auto input = io::File::assoc(stdin);
    std::string name, args_str;
    bool quit = false;

    fmt::print(">>> ");
    if (!input.getword(name) || !input.getline(args_str))
        return true;

    try {
        if (!name.empty()) {
            last_name = name;
            last_args = str::split(args_str, ' ');
        }
        util::call_command(last_name, last_args, parse_error_message,
            Command{ "help",        "h",    &CliDebugger::help,                 this },
            Command{ "continue",    "c",    &CliDebugger::continue_exec,        this },
            Command{ "runframe",    "nmi",  &CliDebugger::runframe,             this },
            Command{ "next",        "n",    &CliDebugger::next,                 this },
            Command{ "step",        "s",    &CliDebugger::step,                 this },
            Command{ "break",       "b",    &CliDebugger::breakpoint,           this },
            Command{ "break",       "b",    &CliDebugger::breakpoint_range,     this },
            Command{ "delbreak",    "db",   &CliDebugger::delete_break,         this },
            Command{ "listbreak",   "lb",   &CliDebugger::list_breakpoints,     this },
            Command{ "status",      "st",   &CliDebugger::status,               this },
            Command{ "status",      "st",   &CliDebugger::status_cpu,           this },
            Command{ "read",        "rd",   &CliDebugger::read_addr,            this },
            Command{ "read",        "rd",   &CliDebugger::read_addr_ram,        this },
            Command{ "write",       "wr",   &CliDebugger::write_addr,           this },
            Command{ "write",       "wr",   &CliDebugger::write_addr_ram,       this },
            Command{ "block",       "bl",   &CliDebugger::block_ram,            this },
            Command{ "block",       "bl",   &CliDebugger::block,                this },
            Command{ "disassemble", "dis",  &CliDebugger::disassemble,          this },
            Command{ "disblock",    "dsb",  &CliDebugger::disassemble_block,    this },
            Command{ "trace",       "tr",   &CliDebugger::trace,                this },
            Command{ "stoptrace",   "str",  &CliDebugger::stop_tracing,         this },
            Command{ "reset",       "r",    &CliDebugger::reset,                this },
            Command{ "quit",        "q",    [&]() { quit = true; }                   }
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
        fmt::print("Breakpoint #{} reached.\n", ev.point_id);
        print_instr();
        break;
    case Debugger::Event::Type::InvalidInstruction:
        fmt::print("Found invalid instruction {:02X} at {:04X}.\n", ev.inv.id, ev.inv.addr);
        break;
    }
}

void CliDebugger::print_instr() const
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

} // namespace debugger

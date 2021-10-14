#include <emu/debugger/debugger.hpp>

#include <algorithm>
#include <emu/core/emulator.hpp>
#include <emu/core/instrinfo.hpp>
#include <emu/util/file.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/utility.hpp>

namespace Debugger {

std::optional<MemorySource> string_to_memsource(const std::string &str)
{
    std::unordered_map<std::string, MemorySource> srcmap = {
        { "",       MemorySource::RAM },
        { "cpu",    MemorySource::RAM },
        { "ram",    MemorySource::RAM },
        { "ppu",    MemorySource::VRAM },
        { "vram",   MemorySource::VRAM },
        { "oam",    MemorySource::OAM },
    };
    return util::map_lookup(srcmap, str);
}

Debugger::Debugger(core::Emulator *e)
    : emu(e), cpudbg(&emu->cpu), ppudbg(&emu->ppu)
{
    emu->on_cpu_error([this](uint8 id, uint16 addr)
    {
        got_error = true;
        Event ev;
        ev.tag = Event::Tag::INV_INSTR;
        ev.inv.id = id;
        ev.inv.addr = addr;
        report_callback(std::move(ev));
    });
}

void Debugger::run(StepType step_type)
{
    const auto test_breakpoints = [this]()
    {
        const uint16 pc = cpudbg.getreg(CPUDebugger::Reg::PC);
        auto it = std::find_if(break_list.begin(), break_list.end(),
                [pc](const Breakpoint &b)
                {
                    switch (b.mode) {
                    case 'x': return pc >= b.start && pc <= b.end;
                    default: return false;
                    }
                });
        return it == break_list.end() ? -1 : it - break_list.begin();
    };

    const auto runloop = [&](auto &&check_step)
    {
        Event ev;
        for (;;) {
            emu->run();
            trace();
            if (got_error)
                break;
            int bhit = test_breakpoints();
            if (bhit != -1) {
                ev.tag = Event::Tag::BREAK;
                ev.bp_index = bhit;
                break;
            }
            if (check_step()) {
                ev.tag = Event::Tag::STEP;
                break;
            }
        }
        report_callback(std::move(ev));
    };

    switch (step_type) {
    case StepType::STEP:
        runloop([]() { return true; });
        break;
    case StepType::NEXT: {
        int cont = 0;
        uint8 id = cpudbg.curr_instr().id;
        runloop(
            [&]()
            {
                switch (id) {
                case 0x20: cont++; break;
                case 0x60: cont--; break;
                }
                id = cpudbg.curr_instr().id;
                return cont <= 0;
            });
        break;
    }
    case StepType::FRAME: {
        uint16 addr = cpudbg.get_vector_addr(core::NMI_VEC);
        runloop([&]() { return cpudbg.getreg(CPUDebugger::Reg::PC) == addr; });
        break;
    }
    case StepType::NONE:
        runloop([]() { return false; });
        break;
    }
}

uint8 Debugger::read_ram(uint16 addr)
{
    return (addr >= 0x2000 && addr <= 0x3FFF) ? ppudbg.getreg(0x2000 + (addr & 0x7))
                                              : emu->rambus.read(addr);
}

std::function<uint8(uint16)> Debugger::read_from(MemorySource source)
{
    switch (source) {
    case MemorySource::RAM:  return util::member_fn(this, &Debugger::read_ram);
    case MemorySource::VRAM: return [&](uint16 addr) { return emu->vrambus.read(addr); };
    case MemorySource::OAM:  return util::member_fn(&ppudbg, &PPUDebugger::read_oam);
    default: panic("get_read_fn");
    }
}

std::function<void(uint16, uint8)> Debugger::write_to(MemorySource source)
{
    switch (source) {
    case MemorySource::RAM:  return [&](uint16 addr, uint8 data) { return emu->rambus.write(addr, data); };
    case MemorySource::VRAM: return [&](uint16 addr, uint8 data) { return emu->vrambus.write(addr, data); };
    case MemorySource::OAM:  return util::member_fn(&ppudbg, &PPUDebugger::write_oam);
    default: panic("get_write_fn");
    }
}

unsigned Debugger::set_breakpoint(Breakpoint &&bp)
{
    auto it = std::find_if(break_list.begin(), break_list.end(), [](const auto &b) {
        return b.mode == 'n';
    });
    if (it != break_list.end()) {
        *it = bp;
        return it - break_list.begin();
    }
    break_list.push_back(bp);
    return break_list.size() - 1;
}

void Debugger::delete_breakpoint(unsigned index)
{
    break_list[index].mode = 'n';
}

void Debugger::trace()
{
    if (!tracefile)
        return;
    fmt::print(tracefile.value().data(),
        "PC: ${:04X} A: ${:02X} X: ${:02X} Y: ${:02X} SP: ${:02X} {} "
        "V: {:04X}"
        " {}\n",
        cpudbg.getreg(CPUDebugger::Reg::PC),
        cpudbg.getreg(CPUDebugger::Reg::ACC),
        cpudbg.getreg(CPUDebugger::Reg::X),
        cpudbg.getreg(CPUDebugger::Reg::Y),
        cpudbg.getreg(CPUDebugger::Reg::SP),
        cpudbg.curr_flags_str(),
        ppudbg.getreg(PPUDebugger::Reg::PPUADDR),
        cpudbg.curr_instr_str()
    );
}

bool Debugger::start_tracing(std::string_view pathname)
{
    tracefile = io::File::open(pathname, io::Access::WRITE);
    return !!tracefile;
}

void Debugger::stop_tracing()
{
    tracefile = std::nullopt;
}

} // namespace Debugger


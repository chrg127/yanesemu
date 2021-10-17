#include <emu/debugger/debugger.hpp>

#include <algorithm>
#include <emu/core/emulator.hpp>
#include <emu/core/instrinfo.hpp>
#include <emu/util/file.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/utility.hpp>

namespace debugger {

std::optional<MemorySource> string_to_memsource(std::string_view str)
{
    if (str.empty())   return MemorySource::RAM;
    if (str == "cpu")  return MemorySource::RAM;
    if (str == "ram")  return MemorySource::RAM;
    if (str == "ppu")  return MemorySource::VRAM;
    if (str == "vram") return MemorySource::VRAM;
    if (str == "oam")  return MemorySource::OAM;
    return std::nullopt;
}

Debugger::Debugger(core::Emulator *e)
    : emu(e), cpudbg(&emu->cpu), ppudbg(&emu->ppu)
{
    emu->on_cpu_error([this](uint8 id, uint16 addr)
    {
        got_error = true;
        report_callback((Event) {
                .type = Event::Type::InvalidInstruction,
                .inv = { .id = id, .addr = addr, },
        });
    });
}

void Debugger::run(StepType step_type)
{
    const auto test_breakpoints = [this]() -> std::optional<unsigned>
    {
        uint16 pc = cpudbg.getreg(CPUDebugger::Reg::PC);
        auto it = std::find_if(break_list.begin(), break_list.end(), [pc](const auto &b) {
            return pc >= b.start && pc <= b.end;
        });
        if (it == break_list.end())
            return std::nullopt;
        return it - break_list.begin();
    };

    const auto runloop = [&](auto &&check_step)
    {
        for (;;) {
            emu->run();
            trace();
            if (got_error)
                break;
            auto hit = test_breakpoints();
            if (hit) {
                report_callback((Event) {
                    .type = Event::Type::Break,
                    .point_id = hit.value(),
                });
                break;
            }
            if (check_step()) {
                report_callback((Event) { .type = Event::Type::Step, .point_id = 0 });
                break;
            }
        }
    };

    switch (step_type) {
    case StepType::Step:
        runloop([]() { return true; });
        break;
    case StepType::Next: {
        int cont = 0;
        uint8 id = cpudbg.curr_instr().id;
        runloop([&]() {
            if (id == 0x20) cont++;
            if (id == 0x60) cont--;
            id = cpudbg.curr_instr().id;
            return cont <= 0;
        });
        break;
    }
    case StepType::Frame: {
        uint16 addr = cpudbg.get_vector_addr(core::NMI_VEC);
        runloop([&]() { return cpudbg.getreg(CPUDebugger::Reg::PC) == addr; });
        break;
    }
    case StepType::None:
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

unsigned Debugger::set_breakpoint(Breakpoint point)
{
    auto it = std::find_if(break_list.begin(), break_list.end(), [](const auto &b) {
        return b.erased;
    });
    if (it != break_list.end()) {
        *it = point;
        return it - break_list.begin();
    }
    break_list.push_back(point);
    return break_list.size() - 1;
}

void Debugger::trace()
{
    if (!tracefile)
        return;
    fmt::print(tracefile.value().data(),
        "PC: ${:04X} A: ${:02X} X: ${:02X} Y: ${:02X} SP: ${:02X} {} V: {:04X} {}\n",
        cpudbg.getreg(CPUDebugger::Reg::PC),
        cpudbg.getreg(CPUDebugger::Reg::Acc),
        cpudbg.getreg(CPUDebugger::Reg::X),
        cpudbg.getreg(CPUDebugger::Reg::Y),
        cpudbg.getreg(CPUDebugger::Reg::SP),
        cpudbg.curr_flags_str(),
        ppudbg.getreg(PPUDebugger::Reg::PPUAddr),
        cpudbg.curr_instr_str()
    );
}

} // namespace Debugger


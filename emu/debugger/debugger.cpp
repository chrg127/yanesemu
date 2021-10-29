#include <emu/debugger/debugger.hpp>

#include <algorithm>
#include <emu/core/emulator.hpp>
#include <emu/platform/input.hpp>
#include <emu/util/file.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/utility.hpp>

namespace debugger {

std::optional<MemorySource> string_to_memsource(std::string_view str)
{
    if (str.empty() || str == "cpu" || str == "ram") return MemorySource::RAM;
    if (str == "ppu" || str == "vram")               return MemorySource::VRAM;
    if (str == "oam")                                return MemorySource::OAM;
    return std::nullopt;
}

std::optional<Component> string_to_component(std::string_view str)
{
    if (str.empty()) return Component::CPU;
    if (str == "cpu") return Component::CPU;
    if (str == "ppu") return Component::PPU;
    return std::nullopt;
}

std::optional<input::Button> string_to_button(std::string_view str)
{
    if (str == "a" || str == "A") return input::Button::A;
    if (str == "b" || str == "B") return input::Button::B;
    if (str == "select")          return input::Button::Select;
    if (str == "start")           return input::Button::Start;
    if (str == "up")              return input::Button::Up;
    if (str == "down")            return input::Button::Down;
    if (str == "left")            return input::Button::Left;
    if (str == "right")           return input::Button::Right;
    return std::nullopt;
}

std::optional<CPUDebugger::Reg> string_to_cpu_reg(std::string_view str)
{
    if (str == "a"  || str == "A")  return CPUDebugger::Reg::Acc;
    if (str == "x"  || str == "X")  return CPUDebugger::Reg::X;
    if (str == "y"  || str == "Y")  return CPUDebugger::Reg::Y;
    if (str == "pc" || str == "PC") return CPUDebugger::Reg::PC;
    if (str == "sp" || str == "SP") return CPUDebugger::Reg::SP;
    if (str == "flags")             return CPUDebugger::Reg::Flags;
    return std::nullopt;
}

std::optional<PPUDebugger::Reg> string_to_ppu_reg(std::string_view str)
{
    if (str == "ctrl"    || str == "PPUCTRL")   return PPUDebugger::Reg::Ctrl;
    if (str == "mask"    || str == "PPUMASK")   return PPUDebugger::Reg::Mask;
    if (str == "status"  || str == "PPUSTATUS") return PPUDebugger::Reg::Status;
    if (str == "oamaddr" || str == "OAMADDR")   return PPUDebugger::Reg::OAMAddr;
    if (str == "oamdata" || str == "OAMDATA")   return PPUDebugger::Reg::OAMData;
    if (str == "scroll"  || str == "PPUSCROLL") return PPUDebugger::Reg::PPUScroll;
    if (str == "addr"    || str == "PPUADDR")   return PPUDebugger::Reg::PPUAddr;
    if (str == "data"    || str == "PPUDATA")   return PPUDebugger::Reg::PPUData;
    return std::nullopt;
}

unsigned BreakList::add(Breakpoint point)
{
    auto it = std::find_if(list.begin(), list.end(), [](const auto &p) { return p.erased; });
    if (it != list.end()) {
        *it = point;
        return it - list.begin();
    }
    list.push_back(point);
    return list.size() - 1;
}

std::optional<unsigned> BreakList::test(u16 addr)
{
    auto it = std::find_if(list.begin(), list.end(), [addr](const auto &p) {
        return addr >= p.start && addr <= p.end;
    });
    if (it == list.end())
        return std::nullopt;
    return it - list.begin();
}

void Tracer::trace(CPUDebugger &cpudbg, PPUDebugger &ppudbg)
{
    if (file) {
        fmt::print(file.value().data(),
            "PC: ${:04X} A: ${:02X} X: ${:02X} Y: ${:02X} SP: ${:02X} {} V: {:04X} {}\n",
                   cpudbg.reg(CPUDebugger::Reg::PC),
                   cpudbg.reg(CPUDebugger::Reg::Acc),
                   cpudbg.reg(CPUDebugger::Reg::X),
                   cpudbg.reg(CPUDebugger::Reg::Y),
                   cpudbg.reg(CPUDebugger::Reg::SP),
                   cpudbg.flags_to_string(),
                   ppudbg.reg(PPUDebugger::Reg::PPUAddr),
                   cpudbg.curr_instr_to_string()
        );
    }
}

Debugger::Debugger()
    : emu(&core::emulator), cpu(&emu->cpu), ppu(&emu->ppu)
{
    emu->on_cpu_error([this](u8 id, u16 addr)
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
    const auto test_breakpoints = [this]() {
        return break_list.test(cpu.reg(CPUDebugger::Reg::PC));
    };

    const auto runloop = [&](auto &&check_step)
    {
        for (;;) {
            emu->run();
            tracer.trace(cpu, ppu);
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
        u8 id = cpu.curr_instr().id;
        runloop([&]() {
            if (id == 0x20) cont++;
            if (id == 0x60) cont--;
            id = cpu.curr_instr().id;
            return cont <= 0;
        });
        break;
    }
    case StepType::Frame: {
        u16 addr = cpu.vector_address(core::NMI_VEC);
        runloop([&]() { return cpu.reg(CPUDebugger::Reg::PC) == addr; });
        break;
    }
    case StepType::None:
        runloop([]() { return false; });
        break;
    }
}

u8 Debugger::read_ram(u16 addr)
{
    return (addr >= 0x2000 && addr <= 0x3FFF) ? ppu.reg(0x2000 + (addr & 0x7))
                                              : emu->rambus.read(addr);
}

std::function<u8(u16)> Debugger::read_from(MemorySource source)
{
    switch (source) {
    case MemorySource::RAM:  return util::member_fn(this, &Debugger::read_ram);
    case MemorySource::VRAM: return [&](u16 addr) { return emu->vrambus.read(addr); };
    case MemorySource::OAM:  return util::member_fn(&ppu, &PPUDebugger::read_oam);
    default: panic("get_read_fn");
    }
}

std::function<void(u16, u8)> Debugger::write_to(MemorySource source)
{
    switch (source) {
    case MemorySource::RAM:  return [&](u16 addr, u8 data) { return emu->rambus.write(addr, data); };
    case MemorySource::VRAM: return [&](u16 addr, u8 data) { return emu->vrambus.write(addr, data); };
    case MemorySource::OAM:  return util::member_fn(&ppu, &PPUDebugger::write_oam);
    default: panic("get_write_fn");
    }
}

void Debugger::reset_emulator()
{
    emu->power(/* reset = */ true);
    report_callback((Event) { .type = Event::Type::Step, .point_id = 0 });
}

} // namespace Debugger

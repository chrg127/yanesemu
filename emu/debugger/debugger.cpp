#include <emu/debugger/debugger.hpp>

#include <algorithm>
#include <emu/core/emulator.hpp>
#include <emu/core/instrinfo.hpp>
#include <emu/util/file.hpp>
#include <emu/util/debug.hpp>

namespace Debugger {

Debugger::Debugger(Core::Emulator *e)
    : emu(e), cpudbg(&emu->cpu)
{
    // emu->cpu.on_fetch([this](uint16 addr, char mode) { fetch_callback(addr, mode); });
    emu->cpu.on_error([this](uint8 id, uint16 addr)  { error_callback(id, addr); });
}

int Debugger::test_breakpoints()
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
}

void Debugger::run()
{
    const auto runloop = [&](auto &&check_step)
    {
        Event ev;
        for (;;) {
            emu->run();
            trace();
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
        step_type = StepType::NONE;
    };

    switch (step_type) {
    case StepType::STEP:
        runloop([]() { return true; });
        break;
    case StepType::NEXT: {
        int cont = 0;
        uint8 id = cpudbg.curr_instr().id;
        runloop([&]()
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
        uint16 addr = cpudbg.get_vector_addr(Core::NMI_VEC);
        runloop([&]() { return cpudbg.getreg(CPUDebugger::Reg::PC) == addr; });
        break;
    }
    case StepType::NONE:
        runloop([]() { return false; });
        break;
    }
}

void Debugger::step()
{
    step_type = StepType::STEP;
    run();
}

void Debugger::next()
{
    step_type = StepType::NEXT;
    run();
}

void Debugger::advance()
{
    step_type = StepType::NONE;
    run();
}

void Debugger::advance_frame()
{
    step_type = StepType::FRAME;
    run();
}

uint8 Debugger::readmem(uint16 addr)
{
    if (addr >= 0x2000 && addr <= 0x2007)
        return emu->ppu.readreg_no_sideeff(addr);
    return emu->rambus.read(addr);
}

void Debugger::writemem(uint16 addr, uint8 value)
{
    emu->rambus.write(addr, value);
}

unsigned Debugger::set_breakpoint(Breakpoint &&bp)
{
    auto it = std::find_if(break_list.begin(), break_list.end(),
            [](const Breakpoint &b) { return b.mode == 'n'; });
    if (it != break_list.end()) {
        *it = bp;
        return it - break_list.begin();
    }
    break_list.push_back(bp);
    return break_list.size() - 1;
}

void Debugger::fetch_callback(uint16 addr, char mode)
{
}

void Debugger::error_callback(uint8 id, uint16 addr)
{
    Event ev;
    ev.tag = Event::Tag::INV_INSTR;
    ev.inv.id = id;
    ev.inv.addr = addr;
    report_callback(std::move(ev));
}

void Debugger::trace()
{
    if (!tracefile)
        return;
    fmt::print(tracefile.data(),
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

} // namespace Debugger


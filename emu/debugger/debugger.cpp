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
    emu->cpu.on_fetch([this](uint16 addr, char mode) { fetch_callback(addr, mode); });
    emu->cpu.on_error([this](uint8 id, uint16 addr)  { error_callback(id, addr); });
}

void Debugger::run()
{
    const auto report_break = [this]() {
        Event ev;
        ev.tag = Event::Tag::BREAK;
        ev.bp_index = break_hit;
        report_callback(std::move(ev));
    };
    const auto report_step = [this]() {
        Event ev;
        ev.tag = Event::Tag::STEP;
        report_callback(std::move(ev));
        steptype = Step::NONE;
    };

    CPUDebugger::Instruction last_instr = cpudbg.curr_instr();
    uint16 last_pc = cpudbg.getreg(CPUDebugger::Reg::PC);

    for (;;) {
        emu->run();
        if (break_hit != -1) {
            break_hit = -1;
            report_break();
            return;
        }
        switch (steptype) {
        case Step::STEP:
            report_step();
            return;
        // Not a very good implementation of a next command, I admit.
        // But a perfect one is quite complex.
        case Step::NEXT:
            if (cpudbg.getreg(CPUDebugger::Reg::PC) == last_pc + Core::num_bytes(last_instr.id)) {
                report_step();
                return;
            }
            break;
        case Step::FRAME: {
            // Check if we're at the NMI handler
            uint16 addr = readmem(Core::NMI_VEC+1) << 8 | readmem(Core::NMI_VEC);
            if (cpudbg.getreg(CPUDebugger::Reg::PC) == addr) {
                report_step();
                return;
            }
            break;
        }
        case Step::NONE:
            break;
        }
    }
}

void Debugger::step()
{
    steptype = Step::STEP;
    run();
}

void Debugger::next()
{
    steptype = Step::NEXT;
    run();
}

void Debugger::runframe()
{
    steptype = Step::FRAME;
    run();
}

void Debugger::continue_exec()
{
    steptype = Step::NONE;
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

void Debugger::start_tracing(Util::File &&f)
{
    stop_tracing();
    tracefile = std::move(f);
}

void Debugger::fetch_callback(uint16 addr, char mode)
{
    if (mode == 'x' && tracefile)
        trace();

    // Check if we've reached a breakpoint
    auto it = std::find_if(breakvec.begin(), breakvec.end(),
                  [addr, mode](const Breakpoint &b) { return b.test(addr, mode); });
    if (it != breakvec.end())
        break_hit = it - breakvec.begin();
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


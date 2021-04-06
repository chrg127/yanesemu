#include <emu/debugger/debugger.hpp>

#include <algorithm>
#include <emu/core/emulator.hpp>
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
    const auto report_step = [this]() {
        Event ev;
        ev.tag = Event::Tag::STEP;
        report_callback(std::move(ev));
        steptype = Step::NONE;
    };

    for (;;) {
        emu->run();
        if (break_hit != -1) {
            break_hit = -1;
            return;
        }
        switch (steptype) {
        case Step::STEP:
            report_step();
            return;
        case Step::NEXT:
            report_step();
            return;
        case Step::FRAME:
            report_step();
            return;
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
    // tracing
    if (mode == 'x' && tracefile)
        trace();

    // check if we've reached a breakpoint
    auto it = std::find_if(breakvec.begin(), breakvec.end(),
                  [addr, mode](const Breakpoint &b) { return b.test(addr, mode); });
    if (it != breakvec.end()) {
        break_hit = it - breakvec.begin();
        Event ev;
        ev.tag = Event::Tag::BREAK;
        ev.bp_index = break_hit;
        report_callback(std::move(ev));
    }
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


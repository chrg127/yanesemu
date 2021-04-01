#include <emu/core/debugger.hpp>

#include <algorithm>
#include <emu/core/emulator.hpp>
#include <emu/util/file.hpp>
#include <emu/util/debug.hpp>

namespace Core {

void Debugger::fetch_callback(CPU::Status &&st, uint16 addr, char mode)
{
    if (quit)
        return;
    if (!callback) {
        warning("No callback set in the debugger\n");
        return;
    }

    // tracing
    if (mode == 'x') {
        update_backtrace(st);
        trace(st, emu->ppu.status());
    }

    // check if we've reached a breakpoint
    auto it = std::find_if(breakvec.begin(), breakvec.end(), [=](const Breakpoint &p) {
                return p.mode == mode && addr >= p.start && addr <= p.end;
            });
    if (it != breakvec.end()) {
        callback(*this, {
            .type   = Event::Type::BREAK,
            .cpu_st = st,
            .index  = it - breakvec.begin(),
        });
    }

    // handle next/step commands
    if (nextstop) {
        /* If the user has issued a next/step command, but an interrupt happened,
         * modify the next stop address so that we get inside the interrupt handler.
         * The interrupt vector is read 2 times, so make sure we ignore the second read */
        if (mode == 'r' && addr >= 0xFFFA && (addr & 1) == 0)
            nextstop = emu->rambus.read(addr+1) << 8 | emu->rambus.read(addr);
        else if (mode == 'x' && addr == nextstop.value()) {
            nextstop.reset();
            callback(*this, {
                .type   = Event::Type::FETCH,
                .cpu_st = st,
                .index  = 0,
            });
        }
    }
}

void Debugger::step()
{
    nextstop = emu->cpu.nextaddr();
}

void Debugger::next()
{
    uint16 pc = emu->cpu.r.pc.full;
    uint8 id = emu->rambus.read(pc);
    // check for jumps and skip them, otherwise use nextaddr()
    nextstop = is_jump(id) ? pc + num_bytes(id)
                           : emu->cpu.nextaddr();
}

CPU::Status Debugger::cpu_status() const
{
    return emu->cpu.status();
}

PPU::Status Debugger::ppu_status() const
{
    return emu->ppu.status();
}

uint8 Debugger::read(uint16 addr)
{
    if (addr >= 0x2000 && addr <= 0x2007)
        return emu->ppu.readreg_no_sideeff(addr);
    return emu->rambus.read(addr);
}

void Debugger::write(uint16 addr, uint8 value)
{
    emu->rambus.write(addr, value);
}

void Debugger::start_tracing(Util::File &&f)
{
    stop_tracing();
    tracefile = std::move(f);
}

// void Debugger::reset()
// {
// }

void Debugger::update_backtrace(CPU::Status st)
{
    switch (st.instr.id) {
    // jsr, jmp, jmp (ind)
    case 0x20: case 0x4C: case 0x6C:
        btrace.push_back(st);
        break;
    // rts
    case 0x60:
        if (btrace.back().instr.id == 0x20)
            btrace.pop_back();
        break;
    }
}

void Debugger::trace(CPU::Status &st, PPU::Status &&pst)
{
    if (!tracefile)
        return;
    fmt::print(tracefile.data(),
        "PC: ${:04X} A: ${:02X} X: ${:02X} Y: ${:02X} SP: ${:02X} {} "
        "V: {:04X}"
        " {}\n",
        st.regs.pc.full, st.regs.acc, st.regs.x, st.regs.y, st.regs.sp,
        format_flags(st.regs.flags),
        pst.vram.addr.value,
        format_instr(st.instr.id, st.instr.op.low, st.instr.op.high,
                     st.regs.pc.full, st.regs.flags)
    );
}

} // namespace Core


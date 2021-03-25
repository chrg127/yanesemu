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
        trace(st);
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

void Debugger::trace(CPU::Status st)
{
    if (tracefile)
        tracefile.print("{}\n", st.instr.id);
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

// void Debugger::reset()
// {
// }

uint8 Debugger::read(uint16 addr)
{
    /* PPU regs 2002 and 2007 have side effects, so emulate them.
     * for 2007, we always return the read buffer. */
    switch (addr) {
    case 0x2002: return emu->ppu.io.vblank << 7 | emu->ppu.io.sp_zero_hit << 6 | emu->ppu.io.sp_overflow << 5;
    case 0x2007: return emu->ppu.vram.addr < 0x3F00 ? emu->ppu.io.data_buf
                                                    : emu->vrambus.read(emu->ppu.vram.addr);
    default: return emu->rambus.read(addr);
    }
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

} // namespace Core


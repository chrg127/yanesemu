#include <emu/debugger/debugger.hpp>

#include <fmt/core.h>
#include <emu/core/cpu.hpp>
#include <emu/core/instrinfo.hpp>

namespace Debugger {

uint16 CPUDebugger::getreg(Reg reg) const
{
    switch (reg) {
    case Reg::ACC:   return cpu->r.acc;
    case Reg::X:     return cpu->r.x;
    case Reg::Y:     return cpu->r.y;
    case Reg::PC:    return cpu->r.pc.full;
    case Reg::SP:    return cpu->r.sp;
    default:    return 0;
    }
}

void CPUDebugger::setreg(Reg reg, uint16 value)
{
    switch (reg) {
    case Reg::ACC:   cpu->r.acc  = value; break;
    case Reg::X:     cpu->r.x    = value; break;
    case Reg::Y:     cpu->r.y    = value; break;
    case Reg::PC:    cpu->r.pc   = value; break;
    case Reg::SP:    cpu->r.sp   = value; break;
    }
}

bool CPUDebugger::getflag(Flag flag) const
{
    switch (flag) {
    case Flag::CARRY:  return cpu->r.flags.carry;
    case Flag::ZERO:   return cpu->r.flags.zero;
    case Flag::INTDIS: return cpu->r.flags.intdis;
    case Flag::DEC:    return cpu->r.flags.decimal;
    case Flag::OV:     return cpu->r.flags.ov;
    case Flag::NEG:    return cpu->r.flags.neg;
    default: return 0;
    }
}

void CPUDebugger::setflag(Flag flag, bool value)
{
    switch (flag) {
    case Flag::CARRY:  cpu->r.flags.carry   = value; break;
    case Flag::ZERO:   cpu->r.flags.zero    = value; break;
    case Flag::INTDIS: cpu->r.flags.intdis  = value; break;
    case Flag::DEC:    cpu->r.flags.decimal = value; break;
    case Flag::OV:     cpu->r.flags.ov      = value; break;
    case Flag::NEG:    cpu->r.flags.neg     = value; break;
    }
}

uint16 CPUDebugger::get_vector_addr(uint16 vector)
{
    return cpu->bus->read(vector+1) << 8 | cpu->bus->read(vector);
}

CPUDebugger::Instruction CPUDebugger::curr_instr()
{
    return {
        .id = cpu->bus->read(cpu->r.pc.full),
        .lo = cpu->bus->read(cpu->r.pc.full+1),
        .hi = cpu->bus->read(cpu->r.pc.full+2),
    };
}

std::string CPUDebugger::curr_instr_str()
{
    const auto took_branch = [](uint8 id, const Core::CPU::ProcStatus &ps)
    {
        switch (id) {
        case 0x10: return ps.neg == 0;
        case 0x30: return ps.neg == 1;
        case 0x50: return ps.ov == 0;
        case 0x70: return ps.ov == 1;
        case 0x90: return ps.carry == 0;
        case 0xB0: return ps.carry == 1;
        case 0xD0: return ps.zero == 0;
        case 0xF0: return ps.zero == 1;
        default:   return false;
        }
    };

    Instruction instr = curr_instr();
    std::string res = Core::disassemble(instr.id, instr.lo, instr.hi);
    if (Core::is_branch(instr.id)) {
        res += fmt::format(" [{:02X}] [{}]",
                Core::branch_pointer(instr.lo, cpu->r.pc.full),
                took_branch(instr.id, cpu->r.flags) ? "Branch taken" : "Branch not taken");
    }
    return res;
}

std::string CPUDebugger::curr_flags_str()
{
    return fmt::format("{}{}{}{}{}{}{}{}",
        (cpu->r.flags.neg    ) ? 'N' : '.',
        (cpu->r.flags.ov     ) ? 'V' : '.',
        (cpu->r.flags.unused ) ? 'U' : '.',
        (cpu->r.flags.breakf ) ? 'B' : '.',
        (cpu->r.flags.decimal) ? 'D' : '.',
        (cpu->r.flags.intdis ) ? 'I' : '.',
        (cpu->r.flags.zero   ) ? 'Z' : '.',
        (cpu->r.flags.carry  ) ? 'C' : '.'
    );
}

unsigned long CPUDebugger::cycles()
{
    return cpu->cycles();
}

} // namespace Core


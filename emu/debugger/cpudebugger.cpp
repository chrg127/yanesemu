#include <emu/debugger/debugger.hpp>

#include <fmt/core.h>
#include <emu/core/cpu.hpp>
#include <emu/debugger/instrinfo.hpp>

namespace debugger {

u16 CPUDebugger::getreg(Reg reg) const
{
    switch (reg) {
    case Reg::Acc:   return cpu->r.acc;
    case Reg::X:     return cpu->r.x;
    case Reg::Y:     return cpu->r.y;
    case Reg::PC:    return cpu->r.pc.v;
    case Reg::SP:    return cpu->r.sp;
    default:    return 0;
    }
}

void CPUDebugger::setreg(Reg reg, u16 value)
{
    switch (reg) {
    case Reg::Acc: cpu->r.acc  = value; break;
    case Reg::X:     cpu->r.x    = value; break;
    case Reg::Y:     cpu->r.y    = value; break;
    case Reg::PC:    cpu->r.pc   = value; break;
    case Reg::SP:    cpu->r.sp   = value; break;
    }
}

bool CPUDebugger::getflag(Flag flag) const
{
    switch (flag) {
    case Flag::Carry:  return cpu->r.flags.carry;
    case Flag::Zero:   return cpu->r.flags.zero;
    case Flag::IntDis: return cpu->r.flags.intdis;
    case Flag::Dec:    return cpu->r.flags.decimal;
    case Flag::Ov:     return cpu->r.flags.ov;
    case Flag::Neg:    return cpu->r.flags.neg;
    default: return 0;
    }
}

void CPUDebugger::setflag(Flag flag, bool value)
{
    switch (flag) {
    case Flag::Carry: cpu->r.flags.carry   = value; break;
    case Flag::Zero: cpu->r.flags.zero     = value; break;
    case Flag::IntDis: cpu->r.flags.intdis = value; break;
    case Flag::Dec: cpu->r.flags.decimal   = value; break;
    case Flag::Ov: cpu->r.flags.ov         = value; break;
    case Flag::Neg: cpu->r.flags.neg       = value; break;
    }
}

u16 CPUDebugger::get_vector_addr(u16 vector) const
{
    return cpu->bus->read(vector+1) << 8 | cpu->bus->read(vector);
}

CPUDebugger::Instruction CPUDebugger::curr_instr() const
{
    return {
        .id = cpu->bus->read(cpu->r.pc.v),
        .lo = cpu->bus->read(cpu->r.pc.v+1),
        .hi = cpu->bus->read(cpu->r.pc.v+2),
    };
}

std::string CPUDebugger::curr_instr_str() const
{
    auto is_branch      = [](u8 id)            { return (id & 0x1F) == 0x10; };
    auto branch_pointer = [](u8 arg, u8 pc) { return pc + 2 + (int8_t) arg; };

    const auto took_branch = [](u8 id, const auto &flags)
    {
        switch (id) {
        case 0x10: return !flags.neg;
        case 0x30: return  flags.neg;
        case 0x50: return !flags.ov;
        case 0x70: return  flags.ov;
        case 0x90: return !flags.carry;
        case 0xB0: return  flags.carry;
        case 0xD0: return !flags.zero;
        case 0xF0: return  flags.zero;
        default:   return false;
        }
    };

    Instruction instr = curr_instr();
    std::string res = disassemble(instr.id, instr.lo, instr.hi).first;
    if (is_branch(instr.id)) {
        res += fmt::format(" [{:02X}] [{}]",
                branch_pointer(instr.lo, cpu->r.pc.v),
                took_branch(instr.id, cpu->r.flags) ? "Branch taken" : "Branch not taken");
    }
    return res;
}

std::string CPUDebugger::curr_flags_str() const
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

unsigned long CPUDebugger::cycles() const
{
    return cpu->cycles();
}

} // namespace core


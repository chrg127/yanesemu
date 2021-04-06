#include <emu/debugger/debugger.hpp>

#include <emu/core/ppu.hpp>

namespace Debugger {

uint16 PPUDebugger::getreg(Reg reg) const
{
    switch (reg) {
    case PPUDebugger::Reg::CTRL:        return ppu->readreg_no_sideeff(0x2000); break;
    case PPUDebugger::Reg::MASK:        return ppu->readreg_no_sideeff(0x2001); break;
    case PPUDebugger::Reg::STATUS:      return ppu->readreg_no_sideeff(0x2002); break;
    case PPUDebugger::Reg::OAMADDR:     return ppu->readreg_no_sideeff(0x2003); break;
    case PPUDebugger::Reg::OAMDATA:     return ppu->readreg_no_sideeff(0x2004); break;
    case PPUDebugger::Reg::PPUSCROLL:   return ppu->readreg_no_sideeff(0x2005); break;
    case PPUDebugger::Reg::PPUADDR:     return ppu->readreg_no_sideeff(0x2006); break;
    case PPUDebugger::Reg::PPUDATA:     return ppu->readreg_no_sideeff(0x2007); break;
    default: return 0;
    }
}

void PPUDebugger::setreg(Reg reg, uint16 value)
{
}

PPUDebugger::Position PPUDebugger::pos() const
{
    Position res = {
        .cycle = ppu->cycles,
        .line = ppu->lines,
    };
    return res;
}

} // namespace Debugger


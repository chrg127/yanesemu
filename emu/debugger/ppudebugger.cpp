#include <emu/debugger/debugger.hpp>

#include <emu/core/ppu.hpp>

namespace Debugger {

uint16 PPUDebugger::getreg(uint16 addr) const
{
    switch (addr) {
    case 0x2000: return ppu->vram.tmp.nt
                      | ppu->io.vram_inc    << 2
                      | ppu->io.sp_pt_addr  << 3
                      | ppu->io.bg_pt_addr  << 4
                      | ppu->io.sp_size     << 5
                      | ppu->io.ext_bus_dir << 6
                      | ppu->io.nmi_enabled << 7;
    case 0x2001: return ppu->io.grey
                      | ppu->io.bg_show_left << 1
                      | ppu->io.sp_show_left << 2
                      | ppu->io.bg_show      << 3
                      | ppu->io.sp_show      << 4
                      | ppu->io.red          << 5
                      | ppu->io.green        << 6
                      | ppu->io.blue         << 7;
    case 0x2002: return ppu->io.vblank      << 7
                      | ppu->io.sp_zero_hit << 6
                      | ppu->io.sp_overflow << 5;
    case 0x2003: return ppu->io.oam_addr;
    case 0x2004: return 0;
    case 0x2005: return !ppu->io.scroll_latch ? ppu->vram.fine_x     << 5 | ppu->vram.tmp.coarse_x
                                              : ppu->vram.tmp.fine_y << 5 | ppu->vram.tmp.coarse_y;
    case 0x2006: return !ppu->io.scroll_latch ? ppu->vram.tmp & 0xFF : ppu->vram.tmp >> 8 & 0xFF;
    case 0x2007: return ppu->vram.addr < 0x3F00 ? ppu->io.data_buf
                                                : ppu->bus->read(ppu->vram.addr);
    default: return 0xFF;
    }
}

uint16 PPUDebugger::getreg(Reg reg) const
{
    switch (reg) {
    case PPUDebugger::Reg::CTRL:        return getreg(0x2000); break;
    case PPUDebugger::Reg::MASK:        return getreg(0x2001); break;
    case PPUDebugger::Reg::STATUS:      return getreg(0x2002); break;
    case PPUDebugger::Reg::OAMADDR:     return getreg(0x2003); break;
    case PPUDebugger::Reg::OAMDATA:     return getreg(0x2004); break;
    case PPUDebugger::Reg::PPUSCROLL:   return getreg(0x2005); break;
    case PPUDebugger::Reg::PPUADDR:     return getreg(0x2006); break;
    case PPUDebugger::Reg::PPUDATA:     return getreg(0x2007); break;
    default: return 0xFF;
    }
}

// void PPUDebugger::setreg(Reg reg, uint8 data)
// {
// }

PPUDebugger::Position PPUDebugger::pos() const
{
    Position res = {
        .cycle = ppu->cycles,
        .line = ppu->lines,
    };
    return res;
}

} // namespace Debugger


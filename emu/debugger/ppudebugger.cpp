#include <emu/debugger/debugger.hpp>

#include <emu/core/ppu.hpp>

namespace debugger {

u8 PPUDebugger::reg(u16 addr) const
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
    case 0x2003: return ppu->oam.addr;
    case 0x2004: return ppu->oam.mem[ppu->oam.addr];
    case 0x2005: return !ppu->io.scroll_latch ? ppu->vram.fine_x     << 5 | ppu->vram.tmp.coarse_x
                                              : ppu->vram.tmp.fine_y << 5 | ppu->vram.tmp.coarse_y;
    case 0x2006: return !ppu->io.scroll_latch ? ppu->vram.tmp.v & 0xFF
                                              : ppu->vram.tmp.v >> 8 & 0xFF;
    case 0x2007: return ppu->vram.addr.v < 0x3F00 ? ppu->io.data_buf
                                                  : ppu->bus->read(ppu->vram.addr.as_u14());
    default: return 0;
    }
}

u8 PPUDebugger::reg(Reg r) const
{
    switch (r) {
    case PPUDebugger::Reg::Ctrl:        return reg(0x2000); break;
    case PPUDebugger::Reg::Mask:        return reg(0x2001); break;
    case PPUDebugger::Reg::Status:      return reg(0x2002); break;
    case PPUDebugger::Reg::OAMAddr:     return reg(0x2003); break;
    case PPUDebugger::Reg::OAMData:     return reg(0x2004); break;
    case PPUDebugger::Reg::PPUScroll:   return reg(0x2005); break;
    case PPUDebugger::Reg::PPUAddr:     return reg(0x2006); break;
    case PPUDebugger::Reg::PPUData:     return reg(0x2007); break;
    default: return 0xFF;
    }
}

void PPUDebugger::set_reg(Reg r, u16 data)
{
    // same as PPU::writereg, but doesn't write in io.latch and takes care of
    // read-only registers
    switch (r) {
    case PPUDebugger::Reg::Ctrl:
        ppu->vram.tmp.nt     = data & 0x03;
        ppu->io.vram_inc     = data & 0x04;
        ppu->io.sp_pt_addr   = data & 0x08;
        ppu->io.bg_pt_addr   = data & 0x10;
        ppu->io.sp_size      = (data & 0x20) >> 5;
        ppu->io.ext_bus_dir  = data & 0x40;
        ppu->io.nmi_enabled  = data & 0x80;
        break;
    case PPUDebugger::Reg::Mask:
        ppu->io.grey          = data & 0x01;
        ppu->io.bg_show_left  = data & 0x02;
        ppu->io.sp_show_left  = data & 0x04;
        ppu->io.bg_show       = data & 0x08;
        ppu->io.sp_show       = data & 0x10;
        ppu->io.red           = data & 0x20;
        ppu->io.green         = data & 0x40;
        ppu->io.blue          = data & 0x80;
        break;
    case PPUDebugger::Reg::Status:
        ppu->io.vblank         = data & 0x80;
        ppu->io.sp_zero_hit    = data & 0x40;
        ppu->io.sp_overflow    = data & 0x20;
        break;
    case PPUDebugger::Reg::OAMAddr:
        ppu->oam.addr = data;
        break;
    case PPUDebugger::Reg::PPUAddr:
        ppu->vram.addr = data;
        break;
    default:
        break;
    }
}

std::pair<unsigned long, unsigned long> PPUDebugger::pos() const
{
    return std::make_pair(ppu->lines, ppu->cycles);
}

u16 PPUDebugger::nt_base_addr() const { return 0x2000 + ppu->vram.tmp.nt * 0x400; }
u16 PPUDebugger::vram_addr() const    { return ppu->vram.addr.v; }
u16 PPUDebugger::tmp_addr() const     { return ppu->vram.tmp.v; }
u8 PPUDebugger::fine_x() const        { return ppu->vram.fine_x; }

u8 PPUDebugger::read_oam(u8 addr)             { return ppu->oam.mem[addr]; }
void PPUDebugger::write_oam(u8 addr, u8 data) { ppu->oam.mem[addr] = data; }

} // namespace Debugger


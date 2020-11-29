void PPU::Background::power()
{
    patterntab_addr = 0;
    nt_base_addr    = 0;
    show            = 0;
    show_leftmost   = 0;
}

void PPU::Background::reset()
{
    patterntab_addr = 0;
    nt_base_addr    = 0;
    show            = 0;
    show_leftmost   = 0;
}

void PPU::Background::fetch_nt(bool dofetch)
{
    if (!dofetch)
        return;
    latch.nt = ppu.vram.read(0x2000 | (ppu.vram.vaddr & 0x0FFF));
}

void PPU::Background::fetch_attr(bool dofetch)
{
    // 0x23C0 | NN | YYY | XXX
    if (!dofetch)
        return;
    latch.attr = ppu.vram.read(0x23C0
                              | (ppu.vram.vaddr      & 0x0C00)
                              | (ppu.vram.vaddr >> 4 & 0x0038)
                              | (ppu.vram.vaddr >> 2 & 0x0007));
}

void PPU::Background::fetch_lowbg(bool dofetch)
{
    if (!dofetch)
        return;
    latch.lowbg = ppu.vram.read(0x1000*patterntab_addr + latch.nt);
}

void PPU::Background::fetch_highbg(bool dofetch)
{
    if (!dofetch)
        return;
    latch.hibg  = ppu.vram.read(0x1000*patterntab_addr + latch.nt+8);
}

void PPU::Background::shift_run()
{
    shift.bglow   >>= 1;
    shift.bghigh  >>= 1;
    shift.athigh >>= 1;
    shift.athigh |= shift.latchhigh << 7;
    shift.atlow >>= 1;
    shift.atlow |= shift.latchlow  << 7;
}

void PPU::Background::fill_shifts()
{
    uint16_t v = ppu.vram.vaddr;
    shift.bglow  = latch.lowbg << 8 | (shift.bglow  & 0xFF);
    shift.bghigh = latch.hibg  << 8 | (shift.bghigh & 0xFF);
    // TODO: this doesn't do what you think it does.
    uint8_t attr_mask = 0b11 << (~((v >> 1 & 1) | (v >> 6 & 1)))*2;
    shift.latchhigh = latch.attr & attr_mask;
    shift.latchlow  = latch.attr & attr_mask;
}

uint8_t PPU::Background::output_bgpixel()
{
    uint8_t mask    = 1 << ppu.vram.fine_x;
    bool lowbit     = shift.bglow    & mask;
    bool hibit      = shift.bghigh   & mask;
    bool at1        = shift.athigh  & mask;
    bool at2        = shift.atlow  & mask;
    uint8_t pal     = at1   << 1 | at2;
    uint8_t palind  = hibit << 1 | lowbit;
    return getcolor(0, pal, palind);
}


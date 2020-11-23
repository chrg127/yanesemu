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

void PPU::fetch_nt(bool dofetch)
{
    if (dofetch)
        bg.latch.nt = vram.read(0x2000 | (vram.vaddr & 0x0FFF));
    
}

void PPU::fetch_attr(bool dofetch)
{
    if (dofetch) {
        // 0x23C0 | NN | YYY | XXX
        bg.latch.attr = vram.read(0x23C0 | (vram.vaddr      & 0x0C00)
                                         | (vram.vaddr >> 4 & 0x0038)
                                         | (vram.vaddr >> 2 & 0x0007));
    }
}

void PPU::fetch_lowbg(bool dofetch)
{
    if (dofetch)
        bg.latch.lowbg = vram.read(0x1000*bg.patterntab_addr + bg.latch.nt);
}

void PPU::fetch_highbg(bool dofetch)
{
    if (dofetch)
        bg.latch.hibg  = vram.read(0x1000*bg.patterntab_addr + bg.latch.nt+8);
}

void PPU::Background::shift_run()
{
    shift.low   >>= 1;
    shift.high  >>= 1;
    shift.attr1 >>= 1;
    shift.attr1 |= shift.attrhigh_latch << 7;
    shift.attr2 >>= 1;
    shift.attr2 |= shift.attrlow_latch  << 7;
}

void PPU::Background::fill_shifts()
{
    shift.low  = latch.lowbg << 8 | (shift.low  & 0xFF);
    shift.high = latch.hibg  << 8 | (shift.high & 0xFF);
    uint16 v = outer.vram.vaddr;
    uint8 attr_mask = 0b11 << (~((v >> 1 & 1) | (v >> 6 & 1)))*2;
    shift.attrhigh_latch = latch.attr & attr_mask;
    shift.attrlow_latch  = latch.attr & attr_mask;
}

uint8 PPU::output_bgpixel()
{
    uint8 mask    = 1 << vram.fine_x;
    bool lowbit     = bg.shift.low    & mask;
    bool hibit      = bg.shift.high   & mask;
    bool at1        = bg.shift.attr1  & mask;
    bool at2        = bg.shift.attr2  & mask;
    uint8 pal     = at1   << 1 | at2;
    uint8 palind  = hibit << 1 | lowbit;
    return getcolor(0, pal, palind);
}


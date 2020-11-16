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
        bg.latch.lowbg = vram.read(bg.latch.nt);
}

void PPU::fetch_highbg(bool dofetch)
{
    if (dofetch)
        bg.latch.hibg = vram.read(bg.latch.nt+8);
}

void PPU::Background::fill_shifts()
{

}

uint24 PPU::output_bgpixel()
{
    uint8_t mask    = 1 << vram.fine_x;
    bool lowbit     = bg.shift_low    & mask;
    bool hibit      = bg.shift_high   & mask;
    bool at1        = bg.shift_attr1  & mask;
    bool at2        = bg.shift_attr2  & mask;
    uint8_t pal     = at1   << 1 | at2;
    uint8_t palind  = hibit << 1 | lowbit;
    return getcolor(0, pal, palind);
}


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

void PPU::fetch_nt()
{
    bg.latch.nt = vram.read(0x2000 | (vram.vaddr & 0x0FFF));
}

void PPU::fetch_at()
{
    // yyyNNYYYYYXXXXX
    // this is 0x23C0 | NN | YYY | XXX
    // where YYY and XXX are first 3 Y and 3 X
    bg.latch.attr = vram.read(0x23C0 | (vram.vaddr      & 0x0C00)
                                | (vram.vaddr >> 4 & 0x0038)
                                | (vram.vaddr >> 2 & 0x0007));
}

void PPU::fetch_lowbg()
{
    bg.latch.lowbg = vram.read(bg.latch.nt);
}

void PPU::fetch_highbg()
{
    bg.latch.hibg = vram.read(bg.latch.nt+8);
}


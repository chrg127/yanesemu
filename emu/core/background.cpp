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

void PPU::Background::fetch_nt()
{
    internal_latch.nt = vram->read();
}

void PPU::Background::fetch_at()
{
    // yyyNNYYYYYXXXXX
    // this is 0x23C0 | NN | YYY | XXX
    // where YYY and XXX are first 3 Y and 3 X
    internal_latch.at = vram->read(0x23C0 | (vram->addr        & 0x0C00)
                                         | ((vram->addr >> 4) & 0x0038)
                                         | ((vram->addr >> 2) & 0x0007));
}

void PPU::Background::fetch_lowbg()
{
    internal_latch.lowbg = vram->read(internal_latch.nt);
}

void PPU::Background::fetch_highbg()
{
    internal_latch.hibg = vram->read(internal_latch.nt+8);
}

void PPU::Background::cycle(int col)
{
    switch (col % 8) {
    case 1: case 2: fetch_nt(); break;
    case 3: case 4: fetch_at(); break;
    case 5: case 6: fetch_lowbg(); break;
    case 7: case 0:
        fetch_highbg();
        vram->incv();
        break;
    }
    int pixel = 0; /* whatever */
    shift_attr1 >>= 1;
    shift_attr2 >>= 1;
}

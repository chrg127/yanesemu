#ifndef INSIDE_PPU_CPP
#error "This file must only be #include'd by ppu.cpp."
#else

void PPU::VRAM::power()
{
    inc     = 1; // ctrl = 0
    readbuf = 0;
    fine_x  = 0;
    t       = 0;
    v       = 0;
    toggle  = 0;
}

void PPU::VRAM::reset()
{
    inc     = 1; // ctrl = 0
    readbuf = 0;
    fine_x  = 0;
    toggle  = 0;
    // t = unchanged
}

void PPU::VRAM::inc_horzpos()
{
    // if ((v & 0x001F) == 31) {
    //     v &= ~0x001F;
    //     v ^= 0x0400;
    // } else
    //     v += 1;
    /*
     * >>> def getwhole(x):
     * ...     return ((x & 0x400) >> 5) | (x & 0x1F)
     * >>> def recompose(x): return (x & 0x20) << 5 | (x & 0x1F)
     * >>> def incv6(x):
     * ...     t = getwhole(x)+1
     * ...     return (x & ~0x41F) | recompose(t)
     */
    uint8 x = ((v & 0x400) >> 5) | ((v & 0x1F) + 1);
    v = (v & ~0x41F) | ((x & 0x20) << 5) | (x & 0x1F);
}

void PPU::VRAM::inc_vertpos()
{
    if ((v & 0x7000) != 0x7000)
        v += 0x1000;
    else {
        v &= ~0x7000;
        int y = (v & 0x03E0) >> 5;
        if (y == 29) {
            y = 0;
            v ^= 0x0800;
        } else if (y == 31)
            y = 0;
        else
            y += 1;
        v = (v & ~0x03E0) | (y << 5);
    }
}

void PPU::VRAM::copy_horzpos()
{
    v = (t & 0x041F) | (v & ~0x41F);
}

void PPU::VRAM::copy_vertpos()
{
    v = (t & 0x7BE0) | (v & ~0x7EB0);
}

#endif

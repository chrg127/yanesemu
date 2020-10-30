void PPU::OAM::power()
{
    sprsize         = 0;
    patterntab_addr = 0;
    show            = 0;
    show_leftmost   = 0;
    addr            = 0;
}

void PPU::OAM::reset()
{
    sprsize         = 0;
    patterntab_addr = 0;
    show            = 0;
    show_leftmost   = 0;
    // addr = unchanged
}

uint8_t PPU::OAM::read(uint16_t addr)
{
    return 0;
}

void PPU::OAM::write(uint16_t addr, uint8_t data)
{

}


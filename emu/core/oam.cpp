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

// uint8 PPU::OAM::read(uint16 addr)
// {
//     return 0;
// }

// void PPU::OAM::write(uint16 addr, uint8 data)
// {

// }

// uint8 PPU::output_sppixel()
// {
//     return 0;
// }

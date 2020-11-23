#ifndef INSIDE_PPU_CPP
#error "This file can only be #include'd by ppu.cpp."
#else

// static uint24 paltab_2c02[] = {
//     0x545454, 0x001E74, 0x081090, 0x300088, 0x440064, 0x5C0030, 0x540400,
//     0x3C1800, 0x202A00, 0x083A00, 0x004000, 0x003C00, 0x00323C, 0x000000,
//     0x000000, 0x000000,

//     0x989698, 0x084CC4, 0x3032EC, 0x5C1EE4, 0x8814B0, 0xA01464, 0x982220,
//     0x783C00, 0x545A00, 0x287200, 0x087C00, 0x007628, 0x006678, 0x000000,
//     0x000000, 0x000000,

//     0xECEEEC, 0x4C9AEC, 0x787CEC, 0xB062EC, 0xE454EC, 0xEC58B4, 0xEC6A64,
//     0xD48820, 0xA0AA00, 0x74C400, 0x4CD020, 0x38CC6C, 0x38B4CC, 0x3C3C3C,
//     0x000000, 0x000000,

//     0xECEEEC, 0xA8CCEC, 0xBCBCEC, 0xD4B2EC, 0xECAEEC, 0xECAED4, 0xECB4B0,
//     0xE4C490, 0xCCD278, 0xB4DE78, 0xA8E290, 0x98E2B4, 0xA0D6E4, 0xA0A2A0,
//     0x000000, 0x000000,
// };

// void PPU::load_palette()
// {
//     paltab = paltab_2c02;
// }

// palette: 0-3, one of the 4 defined palettes for this frame
// color_index: which color of the palette
// bg_or_sp: select if it's a background palette or a sprite palette
// first 2 values are 2 bits big, not 8
uint8 PPU::getcolor(bool select, uint8 pal, uint8 palind)
{
    // this is a 5 bit number
    uint8 i = select << 4 | pal << 2 | palind;
    uint8 color = vram.read(0x3F00 + i);
    return color;
}

#endif


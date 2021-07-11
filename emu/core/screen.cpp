#include "screen.hpp"

static uint32 pal2C02[] = {
    0x545454FF, 0x001E74FF, 0x081090FF, 0x300088FF,
    0x440064FF, 0x5C0030FF, 0x540400FF, 0x3C1800FF,
    0x202A00FF, 0x083A00FF, 0x004000FF, 0x003C00FF,
    0x00323CFF, 0x000000FF, 0x000000FF, 0x000000FF,

    0x989698FF, 0x084CC4FF, 0x3032ECFF, 0x5C1EE4FF,
    0x8814B0FF, 0xA01464FF, 0x982220FF, 0x783C00FF,
    0x545A00FF, 0x287200FF, 0x087C00FF, 0x007628FF,
    0x006678FF, 0x000000FF, 0x000000FF, 0x000000FF,

    0xECEEECFF, 0x4C9AECFF, 0x787CECFF, 0xB062ECFF,
    0xE454ECFF, 0xEC58B4FF, 0xEC6A64FF, 0xD48820FF,
    0xA0AA00FF, 0x74C400FF, 0x4CD020FF, 0x38CC6CFF,
    0x38B4CCFF, 0x3C3C3CFF, 0x000000FF, 0x000000FF,

    0xECEEECFF, 0xA8CCECFF, 0xBCBCECFF, 0xD4B2ECFF,
    0xECAEECFF, 0xECAED4FF, 0xECB4B0FF, 0xE4C490FF,
    0xCCD278FF, 0xB4DE78FF, 0xA8E290FF, 0x98E2B4FF,
    0xA0D6E4FF, 0xA0A2A0FF, 0x000000FF, 0x000000FF,
};

Screen::Screen()
{
    for (std::size_t x = 0; x < buf.width(); x++)
        for (std::size_t y = 0; y < buf.height(); y++)
            buf[y][x] = 0;
    pal = pal2C02;
}

void Screen::output(unsigned x, unsigned y, uint6 value)
{
    buf[y][x] = pal[value];
}

void Screen::set_palette(Palette palette)
{
    auto getpal = [](Palette palette)
    {
        switch (palette) {
        case Palette::PAL2C02: return pal2C02;
        default: return pal2C02;
        }
    };
    pal = getpal(palette);
}


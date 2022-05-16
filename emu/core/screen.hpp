#pragma once

#include <span>
#include <emu/core/const.hpp>
#include <emu/util/array.hpp>
#include <emu/util/uint.hpp>
#include <emu/backend/video.hpp>

class Screen {
    util::Array2D<backend::RGB, core::SCREEN_WIDTH, core::SCREEN_HEIGHT> buf;
    u32 *pal;

public:
    enum class Palette {
        Pal2C02, Pal2C03, RC2C03B,
    };

    Screen() { set_palette(Palette::2C02); }

    void output(unsigned x, unsigned y, u6 value);
    void set_palette(Palette palette);
    std::span<const u8> to_span()
    {
        return std::span{(const u8 *) buf.data(),
                         core::SCREEN_WIDTH * core::SCREEN_HEIGHT * sizeof(backend::RGB)};
    }
};

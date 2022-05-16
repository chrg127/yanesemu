#pragma once

#include <emu/util/common.hpp>

namespace backend {

enum class TextureFormat {
    RGBA,
    RGB,
};

struct TextureOptions {
    std::size_t width, height;
    TextureFormat fmt;
};

inline std::size_t tex_format_to_bpp(TextureFormat fmt)
{
    switch (fmt) {
    case TextureFormat::RGBA: return 4;
    case TextureFormat::RGB:  return 3;
    default: return 0;
    }
}

template <std::size_t N> struct ColorData;
template <> struct ColorData<3> { union { struct { u8 r, g, b;    }; u8 data[3]; }; };
template <> struct ColorData<4> { union { struct { u8 r, g, b, a; }; u8 data[4]; }; };

template <std::size_t N>
struct Color : ColorData<N> {
    Color() = default;
    Color(u32 value)
    {
        for (auto i = 0u; i < N; i++)
            this->data[i] = value >> ((N - 1 - i) * 8) & 0xff;
    }
    u8 & operator[](std::size_t i) { return this->data[i]; }
};

using RGBA = Color<4>;
using RGB  = Color<3>;

} // namespace backend

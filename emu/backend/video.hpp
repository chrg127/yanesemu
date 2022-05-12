#pragma once

namespace backend {

struct Texture {
    std::size_t id, width, height, bpp;
};

enum class TextureFormat {
    RGBA,
    ARGB,
    RGB,
};

} // namespace backend

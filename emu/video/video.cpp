#include <emu/video/video.hpp>

#include <cassert>
#include <fmt/core.h>
// I wish I didn't have to do this...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#define STB_IMAGE_IMPLEMENTATION
#include <external/stb/stb_image.h>
#pragma GCC diagnostic pop
#include <emu/util/unsigned.hpp>
#include <emu/util/debug.hpp>

#include "opengl.hpp"

namespace Video {

std::optional<Context> Context::create(Type type, std::string_view window_name)
{
    const auto create_ptr = [](Type type)
    {
        switch (type) {
        case Type::OPENGL: return std::make_unique<Video::OpenGL>(); break;
        default:
           panic("unknown type supplied to create_context()\n");
           break;
        }
    };
    auto p = create_ptr(type);
    if (!p->init())
        return std::nullopt;
    Context context;
    context.ptr = std::move(p);
    return context;
}

void Canvas::drawpixel(std::size_t x, std::size_t y, uint32_t color)
{
    auto real_y = th-1 - y;
    auto pos = (real_y * tw + x) * 4;
    assert(pos > 0 && pos < tw * th * 4);
    // this code is probably affected by endianness.
    frame[pos  ] = color >> 24 & 0xFF;
    frame[pos+1] = color >> 16 & 0xFF;
    frame[pos+2] = color >> 8  & 0xFF;
    frame[pos+3] = color       & 0xFF;
}

ImageTexture::ImageTexture(const char *pathname)
{
    int width, height, channels;
    data = stbi_load(pathname, &width, &height, &channels, 0);
    assert(data != nullptr && channels == 4);
    tw = width;
    th = height;
}

ImageTexture::~ImageTexture()
{
    stbi_image_free(data);
}

} // namespace Video

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

bool Context::init(Type type)
{
    switch (type) {
    case Type::OPENGL: ptr = std::make_unique<Video::OpenGL>(); break;
    default:           error("unknown type\n");     break;
    }
    return ptr->init();
        // initialized = true;
}

void Context::reset() { }

void Canvas::drawpixel(std::size_t x, std::size_t y, uint32 color)
{
    auto real_y = tex.height()-1 - y;
    auto pos = (real_y * tex.width() + x) * 4;
    assert(pos > 0 && pos < tex.width() * tex.height() * 4);
    // this code is probably affected by endianness.
    frame[pos  ] = color >> 24 & 0xFF;
    frame[pos+1] = color >> 16 & 0xFF;
    frame[pos+2] = color >> 8  & 0xFF;
    frame[pos+3] = color       & 0xFF;
}

ImageTexture::ImageTexture(const char *pathname, Context &ctx)
{
    int width, height, channels;
    data = stbi_load(pathname, &width, &height, &channels, 0);
    assert(data != nullptr && channels == 4);
    tex = Texture(ctx, width, height, data);
}

ImageTexture::~ImageTexture()
{
    stbi_image_free(data);
}

void ImageTexture::reload(const char *pathname)
{
    int width, height, channels;
    data = stbi_load(pathname, &width, &height, &channels, 0);
    assert(data != nullptr && channels == 4);
    tex.update(width, height, data);
    // tex.reset(
}

void ImageTexture::reset(Context &c)
{
    tex.reset(c, data);
}

} // namespace Video

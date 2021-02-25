#include <emu/video/video.hpp>

#include <cassert>
// I wish I didn't have to do this...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#define STB_IMAGE_IMPLEMENTATION
#include <external/stb/stb_image.h>
#pragma GCC diagnostic pop
#include <fmt/core.h>
#include <emu/util/unsigned.hpp>
#include <emu/util/debug.hpp>

#include "opengl.hpp"

namespace Video {

void Context::reset(Type type)
{
    switch (type) {
    case Type::OPENGL: ptr = std::make_unique<Video::OpenGL>(); break;
    default:           error("unknown type\n");     break;
    }
    ptr->init();
    ptr->dimensions(w, h);
}

void Canvas::drawpixel(unsigned x, unsigned y, uint32 color)
{
    auto real_y = h - y;
    auto pos = (real_y * w + x) * 4;
    // this code is probably affected by endianness.
    frame[pos  ] = color >> 24 & 0xFF;
    frame[pos+1] = color >> 16 & 0xFF;
    frame[pos+2] = color >> 8  & 0xFF;
    frame[pos+3] = color       & 0xFF;
}

ImageTexture::ImageTexture(const char *pathname, Context &ctx)
{
    int channels;
    unsigned char *data = stbi_load(pathname, &width, &height, &channels, 0);
    assert(data != nullptr && channels == 4);
    id = ctx.ptr->create_texture(width, height, data);
    stbi_image_free(data);
}

} // namespace Video

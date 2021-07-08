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

std::optional<Context> Context::create(Type type)
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

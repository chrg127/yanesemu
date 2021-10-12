#include <emu/video/video.hpp>

#include <cassert>
#include <fmt/core.h>
// I wish I didn't have to do this...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <external/stb/stb_image.h>
#pragma GCC diagnostic pop
#include <emu/util/debug.hpp>

#include "opengl.hpp"

namespace video {

std::optional<Context> Context::create(Type type)
{
    const auto create_ptr = [](Type type)
    {
        switch (type) {
        case Type::SDL: return std::make_unique<video::OpenGL>(); break;
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

Texture Context::create_texture(std::string_view pathname)
{
    int width, height, channels;
    unsigned char *data = stbi_load(pathname.data(), &width, &height, &channels, 0);
    assert(data != nullptr && channels == 4);
    Texture tex = create_texture(width, height);
    update_texture(tex, data);
    return tex;
}

} // namespace video

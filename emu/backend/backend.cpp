#include "backend.hpp"

#include <cassert>
#include <fmt/core.h>
// I wish I didn't have to do this...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <external/stb/stb_image.h>
#pragma GCC diagnostic pop
#include <emu/util/debug.hpp>

#include "opengl.hpp"
#include "sdl.hpp"

namespace backend {

std::unique_ptr<Backend> create(BackendOpts opts)
{
    switch (opts.type) {
    case Type::SDL_OpenGL: return std::make_unique< OpenGL>(opts.title, opts.window_width, opts.window_height);
    case Type::SDL:        return std::make_unique<    SDL>(opts.title, opts.window_width, opts.window_height);
    case Type::NoVideo:    return std::make_unique<NoVideoBackend>();
    default:
       panic("unknown type supplied to create_context()\n");
       break;
    }
}

u32 Backend::create_texture(std::string_view pathname)
{
    int width, height, channels;
    unsigned char *data = stbi_load(pathname.data(), &width, &height, &channels, 0);
    assert(data != nullptr);
    u32 tex = create_texture({
        .width  = std::size_t(width),
        .height = std::size_t(height),
        .fmt = channels == 4 ? TextureFormat::RGBA
             : channels == 3 ? TextureFormat::RGB
             : TextureFormat::RGBA
    });
    update_texture(tex, std::span((const u8 *) data, width * height * channels));
    return tex;
}

} // namespace backend

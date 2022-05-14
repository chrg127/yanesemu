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

std::unique_ptr<Backend> create(Type type, std::string_view title, std::size_t width, std::size_t height)
{
    switch (type) {
    case Type::SDL_OpenGL: return std::make_unique< OpenGL>(title, width, height);
    case Type::SDL:        return std::make_unique<    SDL>(title, width, height);
    // case Type::NoVideo:    return std::make_unique<Backend>();
    default:
       panic("unknown type supplied to create_context()\n");
       break;
    }
}

Texture Backend::create_texture(std::string_view pathname)
{
    int width, height, channels;
    unsigned char *data = stbi_load(pathname.data(), &width, &height, &channels, 0);
    assert(data != nullptr && channels == 4);
    Texture tex = create_texture(width, height, TextureFormat::RGBA);
    update_texture(tex, data);
    return tex;
}

} // namespace backend

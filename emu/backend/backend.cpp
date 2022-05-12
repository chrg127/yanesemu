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

Backend Backend::create(Type type, std::string_view title, std::size_t width, std::size_t height)
{
    auto f = [&]() -> std::unique_ptr<Impl> {
        switch (type) {
        case Type::SDL_OpenGL: return std::make_unique<OpenGL>(title, width, height);
        // case Type::SDL:        return std::make_unique<   SDL>(title, width, height);
        case Type::NoVideo:    return std::make_unique<  Impl>();
        default:
           panic("unknown type supplied to create_context()\n");
           break;
        }
    };
    Backend b;
    b.ptr = f();
    return b;
}

Texture Backend::create_texture(std::string_view pathname)
{
    int width, height, channels;
    unsigned char *data = stbi_load(pathname.data(), &width, &height, &channels, 0);
    assert(data != nullptr && channels == 4);
    Texture tex = create_texture(width, height);
    update_texture(tex, data);
    return tex;
}

void Backend::map_keys(const conf::Data &conf)
{
    using namespace std::literals;
    using namespace input;
    for (auto p : { std::pair{"AKey"s,      Button::A},
                    std::pair{"BKey"s,      Button::B},
                    std::pair{"UpKey"s,     Button::Up},
                    std::pair{"DownKey"s,   Button::Down},
                    std::pair{"LeftKey"s,   Button::Left},
                    std::pair{"RightKey"s,  Button::Right},
                    std::pair{"StartKey"s,  Button::Start},
                    std::pair{"SelectKey"s, Button::Select} }) {
        auto entry = util::map_lookup(conf, p.first);
        auto s = entry.value().as<std::string>();
        s.erase(s.begin(), s.begin() + 4);
        map_key(s, p.second);
    }
}

bool Backend::is_pressed(input::Button button)
{
    return curr_keys[button];
}

} // namespace backend
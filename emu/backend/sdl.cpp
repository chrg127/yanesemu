#include "sdl.hpp"

namespace backend {

SDL::SDL(std::string_view title, std::size_t width, std::size_t height)
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(title.data(),
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    rd = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

SDL::~SDL()
{
    SDL_DestroyRenderer(rd);
    SDL_DestroyWindow(window);
}

void SDL::set_title(std::string_view title)
{
    SDL_SetWindowTitle(window, title.data());
}

void SDL::resize(std::size_t width, std::size_t height)
{
    SDL_SetWindowSize(window, width, height);
}

void SDL::poll()
{
    for (SDL_Event ev; SDL_PollEvent(&ev); ) {
        switch (ev.type) {
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            auto btn = keymap.find(ev.key.keysym.sym);
            if (btn == keymap.end())
                continue;
            curr_keys[btn->second] = ev.type == SDL_KEYDOWN;
            break;
        }
        }
    }
}

u32 SDL::create_texture(TextureOptions opts)
{
    auto sdl_fmt = [&]() {
        switch (opts.fmt) {
        case TextureFormat::RGBA: return SDL_PIXELFORMAT_RGBA32;
        case TextureFormat::RGB:  return SDL_PIXELFORMAT_RGB888;
        default:                  return SDL_PIXELFORMAT_RGBA32;
        }
    }();
    SDL_Texture *texture = SDL_CreateTexture(rd, sdl_fmt, SDL_TEXTUREACCESS_STREAMING, opts.width, opts.height);
    textures.push_back((TextureInfo) {
        .ptr = std::unique_ptr<SDL_Texture, decltype(sdl_texture_deleter)>(texture),
        .width = opts.width,
        .height = opts.height,
        .bpp = tex_format_to_bpp(opts.fmt),
    });
    return textures.size() - 1;
}

void SDL::update_texture(u32 id, std::span<const u8> data)
{
    auto &tex = textures[id];
    SDL_UpdateTexture(tex.ptr.get(), nullptr, (const void *) data.data(), tex.width * tex.bpp);
}

void SDL::draw_texture(u32 id, std::size_t x, std::size_t y)
{
    auto &tex = textures[id];
    SDL_RenderCopy(rd, tex.ptr.get(), nullptr, nullptr);
}

void SDL::clear()
{
    SDL_RenderClear(rd);
}

void SDL::draw()
{
    SDL_RenderPresent(rd);
    SDL_Delay(1000/60);
}

void SDL::map_key(const std::string &name, input::Button button)
{
    int key = SDL_GetKeyFromName(name.c_str());
    keymap[key] = button;
}

} // namespace backend

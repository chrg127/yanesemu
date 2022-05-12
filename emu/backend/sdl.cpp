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
    for (auto *tex : textures)
        SDL_DestroyTexture(tex);
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

void SDL::poll(input::ButtonArray &keys)
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
            keys[btn->second] = ev.type == SDL_KEYDOWN;
            break;
        }
        }
    }
}

bool SDL::has_quit() { return quit; }

Texture SDL::create_texture(std::size_t width, std::size_t height)
{
    SDL_Texture *texture = SDL_CreateTexture(rd, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
                                             width, height);
    textures.push_back(texture);
    return (Texture) {
        .id = textures.size() - 1,
        .width = width,
        .height = height,
        .bpp = 0,
    };
}

void SDL::update_texture(Texture &tex, const void *data)
{
    auto ptr = textures[tex.id];
    SDL_UpdateTexture(ptr, nullptr, data, tex.width * 4);
}

void SDL::copy_texture(const Texture &tex, std::size_t x, std::size_t y)
{
    auto ptr = textures[tex.id];
    SDL_RenderCopy(rd, ptr, nullptr, nullptr);
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

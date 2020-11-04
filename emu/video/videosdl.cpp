#include <SDL2/SDL.h>
#include <cstring>
#define DEBUG
#include <emu/utils/debug.hpp>

namespace Video {

class DriverSDL : public Driver {
    SDL_Window *wnd = nullptr;
    SDL_Renderer *renderer = nullptr;
    uint32_t *buffer = nullptr;
    int width = 0, height = 0;
    // SDL_Texture *main_texture = nullptr;
    // int pitch = 0;
    bool buf_changed = false;
    bool isclosed = false;

    // void update_texture();

public:

    ~DriverSDL();
    bool create() override;
    void render() override;
    void clear() override;
    void poll() override;
    uint32_t *getpixels() override;
    bool closed() override
    { return isclosed; }
    int getw() const override
    { return width; }
    int geth() const override
    { return height; }
};

DriverSDL::~DriverSDL()
{
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (wnd)
        SDL_DestroyWindow(wnd);
    wnd = nullptr;
    renderer = nullptr;
    // if (main_texture)
    //     SDL_DestroyTexture(main_texture);
    // main_texture = nullptr;
}

bool DriverSDL::create()
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
        goto err;
    width = 640;
    height = 480;

    wnd = SDL_CreateWindow("Window Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_RENDERER_PRESENTVSYNC);
    if (!wnd)
        goto err;
    renderer = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        goto err;
    buffer = new uint32_t[width*height];
    std::memset(buffer, 0, width*height);
    clear();
    return true;

err:
    error("%s\n", SDL_GetError());
    return false;
}

void DriverSDL::render()
{
    for (int i = 0; i < width*height; i++) {
        SDL_SetRenderDrawColor(renderer, buffer[i] >> 24, buffer[i] >> 16, buffer[i] >> 8, buffer[i]);
        SDL_RenderDrawPoint(renderer, i%width, i/width);
    }
    SDL_RenderPresent(renderer);
}

void DriverSDL::clear()
{
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void DriverSDL::poll()
{
    static SDL_Event ev;
    while (SDL_PollEvent(&ev) != 0) {
        if (ev.type == SDL_QUIT)
            isclosed = true;
    }
}

uint32_t *DriverSDL::getpixels()
{
    return buffer;
}



// void DriverSDL::update_texture()
// {
//     void *tbuf;

//     SDL_LockTexture(main_texture, nullptr, &tbuf, &pitch);
//     std::memcpy(tbuf, buffer, pitch*height);
//     SDL_UnlockTexture(main_texture);
//     buf_changed = true;
// }

} // namespace Video

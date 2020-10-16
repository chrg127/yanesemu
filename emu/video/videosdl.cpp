#include <SDL2/SDL.h>
#include <cstring>
#include <emu/utils/debug.hpp>

namespace Video {

class DriverSDL : public Driver {
    SDL_Window *wnd = nullptr;
    SDL_Texture *main_texture = nullptr;
    SDL_Renderer *renderer = nullptr;
    int width = 0, height = 0;
    int pitch = 0;
    uint8_t *buffer = nullptr;
    bool buf_changed = false;
    bool isclosed = false;

    void update_texture();

public:

    ~DriverSDL();
    bool create() override;
    void render() override;
    void clear() override;
    void poll() override;
    uint8_t *getpixels() override;
    bool closed() override
    { return isclosed; }
    int getw() const override
    { return width; }
    int geth() const override
    { return height; }
};

DriverSDL::~DriverSDL()
{
    if (wnd)
        SDL_DestroyWindow(wnd);
    if (main_texture)
        SDL_DestroyTexture(main_texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    wnd = nullptr;
    main_texture = nullptr;
    renderer = nullptr;
}

bool DriverSDL::create()
{
    void *tbuf;

    SDL_Init(SDL_INIT_VIDEO);
    width = 640;
    height = 480;
    wnd = SDL_CreateWindow("Window Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_RENDERER_PRESENTVSYNC);
    if (!wnd)
        goto err;
    renderer = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        goto err;
    main_texture = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(wnd), SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!main_texture)
        goto err;
    buffer = new uint8_t[width*height];
    SDL_LockTexture(main_texture, nullptr, &tbuf, &pitch);
    std::memcpy(buffer, tbuf, width*height);
    SDL_UnlockTexture(main_texture);
    return true;

err:
    error("%s\n", SDL_GetError());
    return false;
}

void DriverSDL::render()
{
   static SDL_Rect rect = { 0, 0, width, height };
   update_texture();
   SDL_RenderCopy(renderer, main_texture, nullptr, &rect);
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
    SDL_Event ev;
    while (SDL_PollEvent(&ev) != 0) {
        if (ev.type == SDL_QUIT)
            isclosed = true;
    }
}

uint8_t *DriverSDL::getpixels()
{
    return buffer;
}



void DriverSDL::update_texture()
{
    void *tbuf;

    SDL_LockTexture(main_texture, nullptr, &tbuf, &pitch);
    std::memcpy(tbuf, buffer, width*height);
    SDL_UnlockTexture(main_texture);
    buf_changed = true;
}

} // namespace Video

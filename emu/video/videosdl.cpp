#include <SDL2/SDL.h>

namespace Video {

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
    SDL_Init(SDL_INIT_VIDEO);
    width = 800;
    height = 600;
    wnd = SDL_CreateWindow("Window Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_RENDERER_PRESENTVSYNC);
    if (!wnd) {
        error("%s\n", SDL_GetError());
        return false;
    }
    renderer = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        error("%s\n", SDL_GetError());
        return false;
    }
    return true;
}

void DriverSDL::update()
{
}

void DriverSDL::clear()
{
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

bool DriverSDL::poll()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev) != 0) {
        if (ev.type == SDL_QUIT)
            return true;
    }
    return false;
}

}

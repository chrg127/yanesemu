/* these are the dumping gounds for any possible new module.
 * there's nothing really interesting here. go away. */

#include <emu/video/video.hpp>
#include <SDL2/SDL.h>

int main()
{
    Video::Context ctx = Video::Context::create<Video::OpenGL>();
    ctx.init();

    Video::Canvas canv { ctx };
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT)
                running = false;
        }
        ctx.update_screen(canv);
        ctx.draw();
    }
}

/* these are the dumping gounds for any possible new module.
 * there's nothing really interesting here. go away. */

#include <cstdio>
#include <fmt/core.h>
#include <emu/video/video.hpp>
#include <SDL2/SDL.h>

int main()
{
    Video::Context ctx;
    ctx.init(Video::Context::Type::OPENGL);
    // ctx.resize(256, 224);

    Video::ImageTexture imtex { "awesomeface.png", ctx };
    // Video::Canvas canv { ctx, ctx.width(), ctx.height() };
    bool running = true;
    SDL_Event event;
    unsigned x = 0, y = 0;

    while (running) {
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                // canv.drawpixel(event.button.x, event.button.y, 0x00FFFFFF);
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    // fmt::print("{}x{}\n", event.window.data1, event.window.data2);
                    ctx.resize(event.window.data1, event.window.data2);
                }
            }
            if (event.type == SDL_QUIT)
                running = false;
        }
        /*
        for (unsigned i = 0; i < 10; i++) {
            canv.drawpixel(x, y, 0x00FFFFFF);
            if (++x > canv.width()) {
                x = 0;
                if (++y > canv.height())
                    running = false;
            }
        }*/
        // ctx.update_canvas(canv);
        ctx.use_image(imtex);
        ctx.draw();
    }
}

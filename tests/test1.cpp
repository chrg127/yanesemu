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

    Video::Canvas canv { ctx, ctx.window_width(), ctx.window_height() };
    // Video::ImageTexture imtex { "someimage.png", ctx };
    // Video::ImageTexture im2 { "funnyimage.png", ctx };
    bool running = true;
    SDL_Event event;
    unsigned x = 0, y = 0;
    // bool whichtex = 0;

    while (running) {
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                canv.drawpixel(event.button.x, event.button.y, 0x00FFFFFF);
                // imtex.reload("dick.png");
                // whichtex ^= 1;
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
        canv.update();
        // if (whichtex)
        //     imtex.use();
        // else
        //     im2.use();
        ctx.draw();
    }
}

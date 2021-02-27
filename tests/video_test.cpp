#include <SDL2/SDL.h>
#include <emu/video/video.hpp>

void test_images()
{
    Video::Context ctx;
    ctx.init(Video::Context::Type::OPENGL);

    Video::ImageTexture imtex { "someimage.png", ctx };
    Video::ImageTexture im2 { "funnyimage.png", ctx };
    bool running = true;
    bool whichtex = 0;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                whichtex ^= 1;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    ctx.resize(event.window.data1, event.window.data2);
                break;
            }
        }
        if (whichtex)
            imtex.use();
        else
            im2.use();
        ctx.draw();
    }
}

void test_canvas()
{
    Video::Context ctx;
    ctx.init(Video::Context::Type::OPENGL);

    Video::Canvas canv { ctx, ctx.window_width(), ctx.window_height() };
    bool running = true;
    SDL_Event event;
    unsigned int x = 0, y = 0;

    while (running) {
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                // drawing this way will totally break once you resize the
                // window. this is intended.
                // canv.drawpixel(event.button.x, event.button.y, 0x00FFFFFF);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    ctx.resize(event.window.data1, event.window.data2);
                break;
            }
        }
        for (int i = 0; i < 3; i++) {
            canv.drawpixel(x, y, 0xFFFFFFFF);
            x++;
            if (x == canv.width()) {
                x = 0;
                y++;
            }
            if (y == canv.height()) {
                running = false;
                break;
            }
        }
        canv.update();
        ctx.draw();
    }
}

int main()
{
    test_canvas();
}

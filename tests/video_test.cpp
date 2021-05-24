#include <SDL2/SDL.h>
#include <emu/video/video.hpp>

void test_images()
{
    auto ctx = Video::Context::create(Video::Context::Type::OPENGL);
    if (!ctx)
        return;

    Video::ImageTexture imtex = ctx->create_image("someimage.png");
    Video::ImageTexture im2 = ctx->create_image("funnyimage.png");
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
                    ctx->resize(event.window.data1, event.window.data2);
                break;
            }
        }
        if (whichtex)
            ctx->use_texture(imtex);
        else
            ctx->use_texture(im2);
        ctx->draw();
    }
}

void test_canvas()
{
    auto ctx = Video::Context::create(Video::Context::Type::OPENGL);
    if (!ctx)
        return;

    Video::Canvas canv = ctx->create_canvas(ctx->window_width(), ctx->window_height());
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
                    ctx->resize(event.window.data1, event.window.data2);
                break;
            }
        }
        const int NUM_PIXELS = ctx->window_width();
        for (int i = 0; i < NUM_PIXELS; i++) {
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
        ctx->update_canvas(canv);
        ctx->use_texture(canv);
        ctx->draw();
    }
}

int main()
{
    test_canvas();
}

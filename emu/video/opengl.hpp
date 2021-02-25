#ifndef VIDEO_OPENGL_HPP_INCLUDED
#define VIDEO_OPENGL_HPP_INCLUDED

#include <SDL2/SDL.h>
#include <emu/video/video.hpp>

namespace Video {

class OpenGL : public Context::Impl {
    bool initialized = false;
    SDL_Window *window = nullptr;
    unsigned width = Context::DEF_WIDTH;
    unsigned height = Context::DEF_HEIGTH;
    SDL_GLContext context;
    unsigned progid;
    unsigned vbo, vao, ebo;

    void create_program();
    void create_objects();

public:
    ~OpenGL();

    bool init();
    void resize(int width, int height);
    void update_canvas(Canvas &canvas);
    void use_image(ImageTexture &imtex);
    void draw();
    unsigned create_texture(int width, int height, unsigned char *data);
    void dimensions(unsigned &w, unsigned &h) const
    {
        w = width;
        h = height;
    }
};

} // namespace Video

#endif

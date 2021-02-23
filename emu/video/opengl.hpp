#ifndef VIDEO_OPENGL_HPP_INCLUDED
#define VIDEO_OPENGL_HPP_INCLUDED

#include <SDL2/SDL.h>
#include <emu/video/video.hpp>

namespace Video {

class OpenGL : public Context::Impl {
    bool initialized = false;
    SDL_Window *window = nullptr;
    unsigned width = 400;
    unsigned height = 300;
    SDL_GLContext context;
    unsigned progid;
    unsigned vbo, vao, ebo;

    void create_shader();
    void create_objects();

public:
    ~OpenGL();
    bool init();
    void resize(int width, int height);
    void update_screen(Canvas &canvas);
    void draw();
    void create_textures(unsigned ids[2]);
    std::pair<unsigned, unsigned> dimensions() const { return std::make_pair(width, height); }
};

} // namespace Video

#endif

#ifndef VIDEO_OPENGL_HPP_INCLUDED
#define VIDEO_OPENGL_HPP_INCLUDED

#include <SDL2/SDL.h>
#include <emu/video/video.hpp>

namespace Video {

class OpenGL : public Context::Impl {
    bool initialized = false;
    SDL_Window *window = nullptr;
    SDL_GLContext context;
    unsigned progid;
    unsigned vbo, vao, ebo;

    void create_program();
    void create_objects();

public:
    ~OpenGL();

    bool init();
    void resize(int width, int height);
    unsigned create_texture(std::size_t texw, std::size_t texh, unsigned char *data = nullptr);
    void update_texture(unsigned id, std::size_t texw, std::size_t texh, unsigned char *data);
    void use_texture(unsigned id);
    void draw();
};

} // namespace Video

#endif

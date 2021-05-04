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
    ~OpenGL() override;

    bool init() override;
    void resize(int width, int height) override;
    unsigned create_texture(std::size_t texw, std::size_t texh, unsigned char *data = nullptr) override;
    void update_texture(unsigned id, std::size_t texw, std::size_t texh, unsigned char *data) override;
    void use_texture(unsigned id) override;
    void draw() override;
};

} // namespace Video

#endif

#pragma once

#include <SDL2/SDL.h>
#include <emu/video/video.hpp>

namespace Video {

class OpenGL : public Context::Impl {
    bool initialized = false;
    SDL_Window *window = nullptr;
    SDL_GLContext context;
    unsigned progid;
    unsigned vbo, vao, ebo;

public:
    ~OpenGL() override;

    bool init() override;
    void resize(int width, int height) override;
    unsigned create_texture(std::size_t texw, std::size_t texh, const void *data = nullptr) override;
    void update_texture(unsigned id, std::size_t texw, std::size_t texh, const void *data) override;
    void use_texture(unsigned id) override;
    void draw() override;
};

} // namespace Video

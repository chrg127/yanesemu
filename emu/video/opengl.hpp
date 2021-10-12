#pragma once

#include <SDL2/SDL.h>
#include <emu/video/video.hpp>

namespace video {

class OpenGL : public Context::Impl {
    bool initialized = false;
    SDL_Window *window = nullptr;
    SDL_GLContext context;
    unsigned prog_id;
    unsigned vbo, vao, ebo;
    bool quit = false;

public:
    ~OpenGL() override;

    bool init() override;
    void set_title(std::string_view title) override;
    void resize(std::size_t width, std::size_t height) override;
    Texture create_texture(std::size_t width, std::size_t height) override;
    void update_texture(Texture &tex, const void *data) override;
    void draw_texture(const Texture &tex, std::size_t x, std::size_t y) override;
    void clear() override;
    void swap() override;
    void poll() override;
    bool has_quit() override;
};

} // namespace video

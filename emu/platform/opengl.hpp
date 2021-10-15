#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>
#include <emu/platform/video.hpp>
#include <emu/platform/input.hpp>

namespace platform {

class OpenGL : public Video::Impl {
    SDL_Window *window = nullptr;
    SDL_GLContext context;
    unsigned prog_id;
    unsigned vbo, vao, ebo;
    std::unordered_map<int, input::Button> keymap;
    bool quit = false;

public:
    ~OpenGL() override;

    void init(std::size_t width, std::size_t height) override;
    void set_title(std::string_view title) override;
    void resize(std::size_t width, std::size_t height) override;
    void poll(input::Keys &keys) override;
    bool has_quit() override;

    Texture create_texture(std::size_t width, std::size_t height) override;
    void update_texture(Texture &tex, const void *data) override;
    void draw_texture(const Texture &tex, std::size_t x, std::size_t y) override;
    void clear() override;
    void swap() override;

    void map_key(const std::string &name, input::Button button) override;
};

} // namespace platform

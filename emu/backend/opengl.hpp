#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>
#include <emu/backend/backend.hpp>

namespace backend {

class OpenGL : public Backend {
    struct TextureInfo {
        unsigned id;
        std::size_t width, height;
        int fmt;
    };

    SDL_Window *window = nullptr;
    SDL_GLContext context;
    unsigned prog_id;
    unsigned vbo, vao, ebo;
    std::vector<TextureInfo> textures;

public:
    OpenGL(std::string_view title, std::size_t width, std::size_t height);
    ~OpenGL() override;
    void set_title(std::string_view title) override;
    void resize(std::size_t width, std::size_t height) override;
    void poll() override;
    u32 create_texture(TextureOptions opts) override;
    void update_texture(u32 tex, std::span<const u8> data) override;
    void draw_texture(u32 tex, std::size_t x, std::size_t y) override;
    void clear() override;
    void draw() override;
    void map_key(const std::string &name, input::Button button) override;
};

} // namespace backend

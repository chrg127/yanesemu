#pragma once

#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <emu/backend/backend.hpp>

namespace backend {

class SDL : public Backend {
    SDL_Window *window;
    SDL_Renderer *rd;
    std::vector<SDL_Texture *> textures;

public:
    SDL(std::string_view title, std::size_t width, std::size_t height);
    ~SDL();
    void set_title(std::string_view title) override;
    void resize(std::size_t width, std::size_t height) override;
    void poll();
    Texture create_texture(std::size_t width, std::size_t height, TextureFormat fmt) override;
    void update_texture(Texture &tex, const void *data) override;
    void draw_texture(const Texture &tex, std::size_t x, std::size_t y) override;
    void clear() override;
    void draw() override;
    void map_key(const std::string &name, input::Button button) override;
};

} // namespace backend

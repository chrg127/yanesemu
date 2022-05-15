#pragma once

#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <emu/backend/backend.hpp>

namespace backend {

inline auto sdl_texture_deleter = [](SDL_Texture *t) { SDL_DestroyTexture(t); };

class SDL : public Backend {
    struct TextureInfo {
        std::unique_ptr<SDL_Texture, decltype(sdl_texture_deleter)> ptr;
        std::size_t width, height, bpp;
    };

    SDL_Window *window;
    SDL_Renderer *rd;
    std::vector<TextureInfo> textures;

public:
    SDL(std::string_view title, std::size_t width, std::size_t height);
    ~SDL();
    void set_title(std::string_view title) override;
    void resize(std::size_t width, std::size_t height) override;
    void poll();
    u32 create_texture(TextureOptions opts) override;
    void update_texture(u32 tex, std::span<const u8> data) override;
    void draw_texture(u32 tex, std::size_t x, std::size_t y) override;
    void clear() override;
    void draw() override;
    void map_key(const std::string &name, input::Button button) override;
};

} // namespace backend

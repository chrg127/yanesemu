#pragma once

#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <emu/backend/backend.hpp>

namespace backend {

struct SDL : public Backend::Impl {
    SDL_Window *window;
    SDL_Renderer *rd;
    bool quit = false;
    std::vector<SDL_Texture *> textures;
    std::unordered_map<int, input::Button> keymap;

    SDL(std::string_view title, std::size_t width, std::size_t height);
    ~SDL();
    void set_title(std::string_view title);
    void resize(std::size_t width, std::size_t height);
    void poll(input::ButtonArray &keys);
    bool has_quit();

    Texture create_texture(std::size_t width, std::size_t height);
    void update_texture(Texture &tex, const void *data);
    void copy_texture(const Texture &tex, std::size_t x, std::size_t y);
    void clear();
    void draw();

    virtual void map_key(const std::string &name, input::Button button);
};

} // namespace backend

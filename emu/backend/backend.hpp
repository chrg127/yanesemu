#pragma once

#include <memory>
#include <string_view>
#include <optional>
#include <emu/util/utility.hpp>
#include <emu/util/conf.hpp>
#include <emu/backend/input.hpp>
#include <emu/backend/video.hpp>

namespace backend {

enum class Type {
    NoVideo,
    SDL_OpenGL,
    SDL,
};

class Backend {
protected:
    std::unordered_map<int, input::Button> keymap;
    input::Keys curr_keys;
    bool quit = false;

public:
    virtual ~Backend() = default;

    virtual void set_title(std::string_view title) = 0;
    virtual void resize(std::size_t width, std::size_t height) = 0;
    virtual void poll() = 0;
    virtual Texture create_texture(std::size_t width, std::size_t height, TextureFormat fmt) = 0;
    virtual void update_texture(Texture &tex, const void *data) = 0;
    virtual void draw_texture(const Texture &tex, std::size_t x, std::size_t y) = 0;
    virtual void clear() = 0;
    virtual void draw() = 0;
    virtual void map_key(const std::string &name, input::Button button) = 0;

    Texture create_texture(std::string_view path);
    virtual bool has_quit() const { return quit; }
    template <util::ContainerType T>
    void update_texture(Texture &tex, const T &buf)
    {
        update_texture(tex, (const void *) buf.data());
    }

    input::Keys get_curr_keys() const { return curr_keys; }
    bool is_pressed(input::Button button) const { return curr_keys[button]; }
};

std::unique_ptr<Backend> create(Type type, std::string_view title, std::size_t width, std::size_t height);

} // namespace backend

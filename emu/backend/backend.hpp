#pragma once

#include <memory>
#include <string_view>
#include <optional>
#include <span>
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
    virtual u32 create_texture(TextureOptions opts) = 0;
    virtual void update_texture(u32 id, std::span<const u8> data) = 0;
    virtual void draw_texture(u32 id, std::size_t x, std::size_t y) = 0;
    virtual void clear() = 0;
    virtual void draw() = 0;
    virtual void map_key(const std::string &name, input::Button button) = 0;

    u32 create_texture(std::string_view path);
    virtual bool has_quit() const { return quit; }
    input::Keys get_curr_keys() const { return curr_keys; }
    bool is_pressed(input::Button button) const { return curr_keys[button]; }
};

class NoVideoBackend : public Backend {
    virtual void set_title(std::string_view title) { }
    virtual void resize(std::size_t width, std::size_t height) { }
    virtual void poll() { }
    virtual u32 create_texture(TextureOptions opts) { return 0; }
    virtual void update_texture(u32 id, std::span<const u8> data) { }
    virtual void draw_texture(u32 id, std::size_t x, std::size_t y) { }
    virtual void clear() { }
    virtual void draw() { }
    virtual void map_key(const std::string &name, input::Button button) { }
};

struct BackendOpts {
    Type type;
    std::string_view title;
    std::size_t window_width, window_height;
};

std::unique_ptr<Backend> create(BackendOpts opts);

} // namespace backend

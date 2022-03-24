#pragma once

#include <memory>
#include <string_view>
#include <optional>
#include <emu/util/utility.hpp>
#include <emu/util/conf.hpp>
#include <emu/platform/input.hpp>

namespace platform {

struct Texture {
    unsigned id;
    std::size_t width, height;
};

enum class Type {
    NoVideo,
    SDL,
};

struct Video {
    struct Impl {
        virtual ~Impl() = default;
        virtual void init(std::size_t width, std::size_t height) { }
        virtual void set_title(std::string_view title) { }
        virtual void resize(std::size_t width, std::size_t height) { }
        virtual void poll(input::ButtonArray &keys) { }
        virtual bool has_quit() { return false; }

        virtual Texture create_texture(std::size_t width, std::size_t height) { return { .id = 0, .width = 0, .height = 0 }; }
        virtual void update_texture(Texture &tex, const void *data) { }
        virtual void draw_texture(const Texture &tex, std::size_t x, std::size_t y) { }
        virtual void clear() { }
        virtual void swap() { }

        virtual void map_key(const std::string &name, input::Button button) { }
    };

private:
    std::unique_ptr<Impl> ptr = nullptr;
    input::ButtonArray curr_keys;

public:
    static Video create(Type type, std::size_t width, std::size_t height);

    void set_title(std::string_view title)                        { ptr->set_title(title); }
    void resize(int width, int height)                            { ptr->resize(width, height); }
    Texture create_texture(std::size_t width, std::size_t height) { return ptr->create_texture(width, height); }
    void update_texture(Texture &tex, const void *data)           { ptr->update_texture(tex, data); }
    void draw_texture(const Texture &tex, std::size_t x, std::size_t y) { ptr->draw_texture(tex, x, y); }
    void clear()                                                  { ptr->clear(); }
    void swap()                                                   { ptr->swap(); }
    void poll()                                                   { ptr->poll(curr_keys); }
    bool has_quit()                                               { return ptr->has_quit(); }
    void map_key(const std::string &name, input::Button button)   { ptr->map_key(name, button); }

    Texture create_texture(std::string_view pathname);

    template <util::ContainerType T>
    void update_texture(Texture &tex, const T &buf)
    {
        ptr->update_texture(tex, (const void *) buf.data());
    }

    void map_keys(const conf::Data &conf);
    bool is_pressed(input::Button button);
};

} // namespace platform

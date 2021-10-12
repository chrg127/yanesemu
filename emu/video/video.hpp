#pragma once

#include <memory>
#include <string_view>
#include <optional>
#include <emu/util/stlutil.hpp>

namespace video {

struct Texture {
    unsigned id;
    std::size_t width, height;
};

enum class Type {
    SDL,
};

struct Context {
    struct Impl {
        virtual ~Impl() = default;
        virtual bool init() = 0;
        virtual void set_title(std::string_view title) = 0;
        virtual void resize(std::size_t width, std::size_t height) = 0;
        virtual Texture create_texture(std::size_t width, std::size_t height) = 0;
        virtual void update_texture(Texture &tex, const void *data) = 0;
        virtual void draw_texture(const Texture &tex, std::size_t x, std::size_t y) = 0;
        virtual void clear() = 0;
        virtual void swap() = 0;
        virtual void poll() = 0;
        virtual bool has_quit() = 0;
    };

private:
    std::unique_ptr<Impl> ptr = nullptr;

public:
    static std::optional<Context> create(Type type);

    void set_title(std::string_view title)                        { ptr->set_title(title); }
    void resize(int width, int height)                            { ptr->resize(width, height); }
    Texture create_texture(std::size_t width, std::size_t height) { return ptr->create_texture(width, height); }
    void update_texture(Texture &tex, const void *data)           { ptr->update_texture(tex, data); }
    void draw_texture(const Texture &tex, std::size_t x, std::size_t y) { ptr->draw_texture(tex, x, y); }
    void clear()                                                  { ptr->clear(); }
    void swap()                                                   { ptr->swap(); }
    void poll()                                                   { ptr->poll(); }
    bool has_quit()                                               { return ptr->has_quit(); }

    Texture create_texture(std::string_view pathname);

    template <util::ContainerType T>
    void update_texture(Texture &tex, const T &buf)
    {
        ptr->update_texture(tex, (const void *) buf.data());
    }
};

} // namespace video

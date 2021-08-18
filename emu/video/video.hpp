#pragma once

#include <cstring>
#include <memory>
#include <string_view>
#include <optional>
#include <emu/util/uint.hpp>
#include <emu/util/array.hpp>

namespace Video {

class Texture {
protected:
    unsigned id;
    std::size_t tw, th;
public:
    unsigned tid()       { return id; }
    std::size_t width()  { return tw; }
    std::size_t height() { return th; }
    friend class Context;
};

class ImageTexture : public Texture {
    unsigned char *data;
public:
    ImageTexture(const char *pathname);
    ~ImageTexture();
    ImageTexture(const ImageTexture &) = delete;
    ImageTexture(ImageTexture &&) = default;
    ImageTexture & operator=(const ImageTexture &) = delete;
    ImageTexture & operator=(ImageTexture &&) = default;

    friend class Context;
};

struct Context {
    struct Impl {
        virtual ~Impl() = default;
        virtual bool init() = 0;
        virtual void resize(int newwidth, int newheight) = 0;
        virtual unsigned create_texture(std::size_t texw, std::size_t texh, const void *data = nullptr) = 0;
        virtual void update_texture(unsigned id, std::size_t texw, std::size_t texh, const void *data) = 0;
        virtual void use_texture(unsigned id) = 0;
        virtual void draw() = 0;
    };

    enum class Type {
        OPENGL,
    };

    static const unsigned DEF_WIDTH  = 512;
    static const unsigned DEF_HEIGTH = 480;

private:
    std::unique_ptr<Impl> ptr = nullptr;
    unsigned wnd_width = DEF_WIDTH, wnd_height = DEF_HEIGTH;

public:
    Context() = default;
    Context(const Context &) = delete;
    Context(Context &&) = default;
    Context & operator=(const Context &) = delete;
    Context & operator=(Context &&) = default;

    static std::optional<Context> create(Type type);

    unsigned window_width() const            { return wnd_width; }
    unsigned window_height() const           { return wnd_height; }
    void use_texture(const Texture &tex)     { ptr->use_texture(tex.id); }
    void draw()                              { ptr->draw(); }

    void resize(int newwidth, int newheight)
    {
        wnd_width = newwidth;
        wnd_height = newheight;
        ptr->resize(newwidth, newheight);
    }

    Texture create_texture(std::size_t w, std::size_t h)
    {
        Texture tex;
        tex.id = ptr->create_texture(w, h, nullptr);
        tex.tw = w;
        tex.th = h;
        return tex;
    }

    void update_texture(const Texture &tex, const void *data)
    {
        ptr->update_texture(tex.id, tex.tw, tex.th, data);
    }

    ImageTexture create_image(std::string_view pathname)
    {
        ImageTexture image(pathname.data());
        image.id = ptr->create_texture(image.tw, image.th, image.data);
        return image;
    }
};

} // namespace Video

#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include <cstring>
#include <cstdint>
#include <memory>
#include <string_view>
#include <optional>

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

class Canvas : public Texture {
    unsigned char *frame;
public:
    Canvas(std::size_t width, std::size_t height)
        : frame(new unsigned char[width*height*4])
    {
        std::memset(frame, 0, width*height*4);
        tw = width;
        th = height;
    }

    ~Canvas()
    {
        if (frame)
            delete[] frame;
    }

    Canvas(const Canvas &) = delete;
    Canvas(Canvas &&) = default;
    Canvas & operator=(const Canvas &) = delete;
    Canvas & operator=(Canvas &&) = default;

    void drawpixel(std::size_t x, std::size_t y, uint32_t color);

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
        virtual unsigned create_texture(std::size_t texw, std::size_t texh, unsigned char *data = nullptr) = 0;
        virtual void update_texture(unsigned id, std::size_t texw, std::size_t texh, unsigned char *data) = 0;
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

    void update_texture(const Texture &tex, unsigned char *data) { ptr->update_texture(tex.id, tex.tw, tex.th, data); }

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
    void update_canvas(const Canvas &canvas) { update_texture(canvas, canvas.frame); }
    void draw()                              { ptr->draw(); }

    void resize(int newwidth, int newheight)
    {
        wnd_width = newwidth;
        wnd_height = newheight;
        ptr->resize(newwidth, newheight);
    }

    Canvas create_canvas(std::size_t width, std::size_t height)
    {
        Canvas canvas{width, height};
        canvas.id = ptr->create_texture(width, height, canvas.frame);
        return canvas;
    }

    ImageTexture create_image(std::string_view pathname)
    {
        ImageTexture image(pathname.data());
        image.id = ptr->create_texture(image.tw, image.th, image.data);
        return image;
    }
};

} // namespace Video

#endif

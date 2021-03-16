#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include <cstdint>
#include <memory>
#include <string_view>

namespace Video {

class Canvas;
class ImageTexture;

struct Context {
    struct Impl {
        virtual ~Impl() { }
        virtual bool init() = 0;
        virtual void resize(int newwidth, int newheight) = 0;
        // these 3 methods should only be called by Texture
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
    // bool initialized = false;

public:
    Context() = default;
    Context(const Context &) = delete;
    Context(Context &&) = default;
    Context & operator=(const Context &) = delete;
    Context & operator=(Context &&) = default;

    // explicit operator bool() const { return initialized; }

    bool init(Type type);

    void reset();
    void resize(int newwidth, int newheight)
    {
        wnd_width = newwidth;
        wnd_height = newheight;
        ptr->resize(newwidth, newheight);
    }
    void draw()                        { ptr->draw(); }
    unsigned window_width() const      { return wnd_width; }
    unsigned window_height() const     { return wnd_height; }

    friend class Texture;
};

class Texture {
    Context *ctx = nullptr;
    unsigned id = 0;
    std::size_t tw = 0, th = 0;
public:
    Texture() = default;
    Texture(Context &c, std::size_t width, std::size_t height, unsigned char *data = nullptr)
        : ctx(&c), tw(width), th(height)
    {
        id = ctx->ptr->create_texture(tw, th, data);
    }

    void reset(Context &c, unsigned char *data = nullptr)
    {
        ctx = &c;
        id = ctx->ptr->create_texture(tw, th, data);
    }
    unsigned tid() const             { return id; }
    unsigned width() const           { return tw; }
    unsigned height() const          { return th; }
    void update(unsigned char *data) { ctx->ptr->update_texture(id, tw, th, data); }
    void update(std::size_t width, std::size_t height, unsigned char *data)
    {
        tw = width;
        th = height;
        update(data);
    }
    void use()                       { ctx->ptr->use_texture(id); }
};

class Canvas {
    Texture tex;
    unsigned char *frame;
public:
    Canvas(Context &ctx, std::size_t width, std::size_t height)
        : tex(ctx, width, height), frame(new unsigned char[width*height*4])
    { }

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

    unsigned width() const  { return tex.width(); }
    unsigned height() const { return tex.height(); }
    void update()           { tex.update(frame); }
    void reset(Context &c)  { tex.reset(c); }
};

class ImageTexture {
    Texture tex;
    unsigned char *data;
public:
    ImageTexture(const char *pathname, Context &ctx);
    ~ImageTexture();
    ImageTexture(const ImageTexture &) = delete;
    ImageTexture(ImageTexture &&) = default;
    ImageTexture & operator=(const ImageTexture &) = delete;
    ImageTexture & operator=(ImageTexture &&) = default;

    void reload(const char *pathname);
    void reset(Context &c);

    unsigned width() const  { return tex.width(); }
    unsigned height() const { return tex.height(); }
    void use()              { tex.use(); }
};

} // namespace Video

#endif

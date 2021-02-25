#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include <memory>
#include <utility>
#include <string_view>
#include <emu/util/unsigned.hpp>

namespace Video {

class Canvas;
class ImageTexture;

class Context {
public:
    struct Impl {
        virtual ~Impl() { }
        virtual bool init() = 0;
        virtual void resize(int newwidth, int newheight) = 0;
        virtual void update_canvas(Canvas &canvas) = 0;
        virtual void use_image(ImageTexture &imtex) = 0;
        virtual void draw() = 0;
        virtual void dimensions(unsigned &w, unsigned &h) const = 0;
        virtual unsigned create_texture(int texw, int texh, unsigned char *data = nullptr) = 0;
    };

    enum class Type {
        OPENGL,
    };

private:
    std::unique_ptr<Impl> ptr = nullptr;
    unsigned w = 0, h = 0;

public:
    static const unsigned DEF_WIDTH  = 400;
    static const unsigned DEF_HEIGTH = 300;

    Context() = default;
    Context(const Context &) = delete;
    Context(Context &&)      = default;
    Context & operator=(const Context &) = delete;
    Context & operator=(Context &&)      = default;

    void reset(Type type);

    void init(Type type)               { reset(type); }
    void resize(int width, int height) { w = width; h = height; ptr->resize(width, height); }
    void update_canvas(Canvas &canvas) { ptr->update_canvas(canvas); }
    void use_image(ImageTexture &im)   { ptr->use_image(im); }
    void draw()                        { ptr->draw(); }
    unsigned width() const             { return w; }
    unsigned height() const            { return h; }

    friend class Canvas;
    friend class ImageTexture;
};

class Canvas {
    unsigned tex_ids[2];
    unsigned currid = 0;
    unsigned w = 0, h = 0;
    unsigned char *frame = nullptr;

public:
    Canvas(Context &ctx, unsigned width, unsigned height)
        : w(width), h(height), frame(new unsigned char[w * h * 4]())
    {
        tex_ids[0] = ctx.ptr->create_texture(w, h);
    }

    ~Canvas()
    {
        if (frame)
            delete[] frame;
    }

    Canvas(const Canvas &) = delete;
    Canvas(Canvas &&c)     = default;
    Canvas & operator=(const Canvas &) = delete;
    Canvas & operator=(Canvas &&c)     = default;

    void drawpixel(unsigned x, unsigned y, uint32 color);

    unsigned texid() const      { return tex_ids[currid]; }
    unsigned char *get_frame() const { return frame; }
    unsigned width() const          { return w; }
    unsigned height() const         { return h; }
};

class ImageTexture {
    int width, height;
    unsigned id;

public:
    ImageTexture(const char *pathname, Context &ctx);

    unsigned texid() const { return id; }
};

} // namespace Video

#endif

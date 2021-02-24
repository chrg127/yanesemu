#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include <memory>
#include <utility>

namespace Video {

class Canvas;

class Context {
public:
    struct Impl {
        virtual ~Impl() { }
        virtual bool init() = 0;
        virtual void resize(int width, int height) = 0;
        virtual void update_screen(Canvas &canvas) = 0;
        virtual void draw() = 0;
        // these two are only called by Canvas::create(), no equivalent for
        // Context is needed
        virtual std::pair<unsigned, unsigned> dimensions() const = 0;
        virtual void create_textures(unsigned ids[2]) = 0;
    };

private:
    std::unique_ptr<Impl> ptr;

public:
    template <typename T> // T = derived from Context::Impl
    static Context create()
    {
        Context c;
        c.ptr = std::make_unique<T>();
        return c;
    }

    Context() = default;
    Context(const Context &) = delete;
    Context(Context &&)      = default;
    Context & operator=(const Context &) = delete;
    Context & operator=(Context &&)      = default;

    void init()                        { ptr->init(); }
    void resize(int width, int height) { ptr->resize(width, height); }
    void update_screen(Canvas &canvas) { ptr->update_screen(canvas); }
    void draw()                        { ptr->draw(); }
    template <typename T>
    void reset()                       { ptr.reset(new T()); }

    friend class Canvas;
};

// NOTE find a way to make this a "class" instead (i.e. keep its members private)
struct Canvas {
    unsigned tex_ids[2];
    unsigned currid = 0;
    unsigned char *frame = nullptr;

public:
    Canvas(Context &ctx)
    {
        auto dim = ctx.ptr->dimensions();
        ctx.ptr->create_textures(tex_ids);
        frame = new unsigned char[dim.first * dim.second * 4]();
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

    void drawpixel(unsigned x, unsigned y);
};

} // namespace Video


#define USE_OPENGL

#ifdef USE_OPENGL
#include "opengl.hpp"
#endif

#undef USE_OPENGL

#endif

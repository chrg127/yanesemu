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
    enum class Type {
        OPENGL,
    };

private:
    std::unique_ptr<Impl> ptr;

public:
    Context(Type type);
    Context(const Context &) = delete;
    Context(Context &&)      = delete;
    Context & operator=(const Context &) = delete;
    Context & operator=(Context &&)      = delete;

    void init()                        { ptr->init(); }
    void resize(int width, int height) { ptr->resize(width, height); }
    void update_screen(Canvas &canvas) { ptr->update_screen(canvas); }
    void draw()                        { ptr->draw(); }
    friend class Canvas;
};

// NOTE find a way to make this a "class" instead (i.e. keep its members private)
struct Canvas {
    unsigned tex_ids[2];
    unsigned currid = 0;
    unsigned char *frame = nullptr;

public:
    Canvas() = default;
    ~Canvas()
    {
        if (frame)
            delete[] frame;
    }

    Canvas(const Canvas &) = delete;
    Canvas(Canvas &&c) { operator=(std::move(c)); }
    Canvas & operator=(const Canvas &) = delete;
    Canvas & operator=(Canvas &&c)
    {
        std::swap(*this, c);
        return *this;
    }

    void drawpixel(unsigned x, unsigned y);

    static Canvas create(Context &ctx);
};

} // namespace Video

#endif

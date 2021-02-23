#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include <memory>

namespace Video {

enum class ContextType {
    OPENGL,
};

struct Canvas {
    unsigned tex_ids[2];
    unsigned currid;
    unsigned char *frame = nullptr;

    ~Canvas() { if (frame) delete[] frame; }
};

struct Context {
    struct Impl {
        virtual ~Impl() { }
        virtual bool init() = 0;
        virtual void resize(int width, int height) = 0;
        virtual void update_screen(Canvas &canvas) = 0;
        virtual void draw() = 0;
        virtual Canvas create_canvas() = 0;
    };

private:
    std::unique_ptr<Impl> ptr;

public:
    Context(ContextType type);
    Context(const Context &) = delete;
    Context(Context &&)      = delete;
    Context & operator=(const Context &) = delete;
    Context & operator=(Context &&)      = delete;

    void init()                        { ptr->init(); }
    Canvas create_canvas()             { return ptr->create_canvas(); }
    void resize(int width, int height) { ptr->resize(width, height); }
    void update_screen(Canvas &canvas) { ptr->update_screen(canvas); }
    void draw()                        { ptr->draw(); }
};

} // namespace Video

#endif

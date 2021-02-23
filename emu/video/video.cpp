#include <emu/video/video.hpp>

#include <emu/util/debug.hpp>
#include "opengl.hpp"

namespace Video {

Context::Context(Type type)
{
    switch (type) {
    case Type::OPENGL: ptr = std::make_unique<OpenGL>(); break;
    default: error("unknown type\n"); break;
    }
}

Canvas Canvas::create(Context &ctx)
{
    Canvas res;
    auto dim = ctx.ptr->dimensions();
    ctx.ptr->create_textures(res.tex_ids);
    res.frame = new unsigned char[dim.first * dim.second * 4]();
    return res;
}

void Canvas::drawpixel(unsigned x, unsigned y)
{
    
}

} // namespace Video

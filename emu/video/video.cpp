#include <emu/video/video.hpp>

#include <emu/util/debug.hpp>
#include "opengl.hpp"

namespace Video {

Context::Context(ContextType type)
{
    switch (type) {
    case ContextType::OPENGL: ptr = std::make_unique<OpenGL>(); break;
    default: error("unknown type\n"); break;
    }
}

} // namespace Video

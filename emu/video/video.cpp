#include <emu/video/video.hpp>

#ifdef VIDEO_USE_SDL
#include <emu/video/videosdl.cpp>
#endif

namespace Video {

void Video::reset()
{
    if (driver)
        delete driver;
    driver = new DriverSDL();
}

}

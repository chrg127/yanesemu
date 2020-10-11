#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include <cstdint>

namespace Video {

class Driver {
public:
    virtual ~Driver() = default;
    virtual bool create() { return true; };
    virtual void update() {};
    virtual void clear() {};
    virtual bool poll() { return true; };
};

class Video {

};

} // namespace Video

#define VIDEO_USE_SDL

#ifdef VIDEO_USE_SDL
#include <emu/video/videosdl.hpp>
#endif

#endif // VIDEO_HPP_INCLUDED


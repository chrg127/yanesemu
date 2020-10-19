#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include <cstdint>

namespace Video {

struct Driver {
    virtual ~Driver() = default;
    virtual bool create()
    { return true; };
    virtual void render()
    { }
    virtual void clear()
    { }
    virtual void poll()
    { };
    virtual uint32_t *getpixels()
    { return nullptr; }
    virtual int getw() const
    { return 0; }
    virtual int geth() const
    { return 0; }

    virtual bool closed()
    { return true; }
};

class Video {
    Driver *driver = nullptr;

public:
    Video()
    { reset(); }

    ~Video()
    {
        if (driver)
            delete driver;
    }

    struct Screen {
        uint32_t *buf;
        int w, h;
    };

    void reset();
    inline bool create()
    { return driver->create(); }
    inline void render()
    { driver->render(); }
    inline void clear()
    { driver->clear(); }
    inline void poll()
    { driver->poll(); }

    inline Screen getpixels()
    {
        return { driver->getpixels(),
                driver->getw(),
                driver->geth() };
    }

    inline bool closed()
    { return driver->closed(); }
};

} // namespace Video

#define VIDEO_USE_SDL

#endif // VIDEO_HPP_INCLUDED

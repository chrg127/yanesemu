This directory contains the video library for yanesemu. Below you will
find documentation for it.

The yanesemu video library is a small library meant for use by emulators only.
Therefore, it isn't meant to be very general, and will only provide what is
necessary for an emulator to work well.
There are four main entities:
    - Context: a context, or driver, or simply a graphics library, represents
      the underlying graphics library used. Note that it doesn't handle OS
      events, this must be done by yourself (this will probably change in the
      future). It exposes only 4 methods:
      - init(): this will initialize the Context. It is provided as separate to
        the constructor so that any initialization error can be handled without
        use of exceptions.
      - reset(): un-initialize the underlying graphics library. If init() was
        never called, this does nothing. This method is implicitly called by init().
      - resize(): resizes the window. This method should only be called when
        getting the corresponding OS event.
      - draw(): draws whatever Texture is in use at the moment. It is worth
        noting that exactly only one texture can be drawn at any moment.
    - Texture: a Texture is a non-owning reference to a texture inside the
      Context. non-owning means that it will NOT clean up its resources, and
      that those resources will be invalid once a Context object goes out of
      scope/is reset using reset(). A Texture object must be initializated by
      passing the Context object and a width and height. Afterwards, any
      operation on the object will be reflected on the Context. This allows the
      object to be semi-independent, but care must be given in a multi-threading
      environment. A Texture might also be initialized with some data.
      The Texture class exposes the following methods:
      - Accessor method tid(), width() and height(), which return an unique
        identifier for the underlying texture, the width and the height,
        respectively
      - reset(context, data): resets the Texture object with a new Context. It will ask the
        new context to create a new underlying texture.
      - update(data): updates the texture with new data as specified by data. A
        class to this method will also use() the texture (and therefore the
        Context will register the texture to be drawn later).
      - update(width, height, data): same as update, but also update width and
        height. Calls update(data) internally.
      - use(): sets the texture as the current texture to draw.
      A Texture is more of a lower-level class. The following classes should be
      used instead.
    - Canvas: a Canvas represent a place to draw pixels. Width and height are
      specified in the constructor, and can never be changed again. It exposes
      the following methods:
      - drawpixel(x, y, color): draws a pixel at the specified coordinate.
      - update(): updates the underlying texture.
      - reset(context): reset the underlying texture to use a new context.
    - ImageTexture: represents a texture that uses an image. The image is loaded
      when contrusting the object or when using reload(pathname). It exposes
      the following methods:
      - reload(pathname): loads a new image as specified by pathname.
      - use(): sets the underlying texture to be used.
      - reset(context): reset the underlying texture to use a new context.

In tests/video_test.cpp an example of how to use this API can be found.
Extending with new functionality: TODO

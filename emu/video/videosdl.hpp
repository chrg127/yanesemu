struct SDL_Window;
struct SDL_Texture;
struct SDL_Renderer;

namespace Video {

class DriverSDL : public Driver {
    SDL_Window *wnd = nullptr;
    SDL_Texture *main_texture = nullptr;
    SDL_Renderer *renderer = nullptr;
    int width, height;
public:
    ~DriverSDL();
    bool create();
    void update();
    void clear();
    bool poll();
};

}


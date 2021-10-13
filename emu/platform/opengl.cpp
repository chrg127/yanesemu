#include "opengl.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <emu/util/os.hpp>

#ifdef PLATFORM_LINUX
#   define GL_GLEXT_PROTOTYPES
#   include <GL/gl.h>
#   include <GL/glu.h>
#   include <GL/glext.h>
#   undef GL_GLEXT_PROTOTYPES
#else
#   error "platform not supported"
#endif

#include <emu/util/debug.hpp>

static char vertcode[] = R"(
    #version 330 core
    layout (location = 0) in vec3 pos;
    layout (location = 1) in vec2 tex_coords;
    out vec2 coords;
    uniform mat4 model, projection;

    void main()
    {
        gl_Position = projection * model * vec4(pos, 1.0);
        coords = tex_coords;
    }
)";

static char fragcode[] = R"(
    #version 330 core
    out vec4 color;
    in vec2 coords;
    uniform sampler2D tex;

    void main()
    {
        color = texture(tex, coords);
    }
)";

// static float vertices[] = {
// //   vertices            texture coordinates
//      1.0f,  1.0f, 0.0f,  1.0f, 0.0f, // top right
//      1.0f, -1.0f, 0.0f,  1.0f, 1.0f, // bottom right
//     -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, // top left
//     -1.0f, -1.0f, 0.0f,  0.0f, 1.0f, // bottom left
// };

static float vertices[] = {
    // pos               tex coords
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // top right
     1.0f,  0.0f, 0.0f,  1.0f, 0.0f, // bottom right
     0.0f,  1.0f, 0.0f,  0.0f, 1.0f, // top left
     0.0f,  0.0f, 0.0f,  0.0f, 0.0f, // bottom left
};

unsigned indices[] = {
    0, 1, 2,
    1, 2, 3,
};

namespace {

unsigned create_shader(unsigned prog_id, unsigned type, const char *code, const char *name)
{
    unsigned sid = glCreateShader(type);
    glShaderSource(sid, 1, &code, nullptr);
    glCompileShader(sid);
    int result = GL_FALSE;
    glGetShaderiv(sid, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int len = 0;
        glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &len);
        auto infolog = std::make_unique<char[]>(len + 1);
        glGetShaderInfoLog(sid, len, &len, infolog.get());
        infolog[len] = '\0';
        error("OpenGL: {} compile error: {}\n", name, infolog.get());
        return 0;
    }
    glAttachShader(prog_id, sid);
    return sid;
}

unsigned create_program()
{
    unsigned prog_id = glCreateProgram();
    auto vs = create_shader(prog_id, GL_VERTEX_SHADER, vertcode, "vertex shader");
    auto fs = create_shader(prog_id, GL_FRAGMENT_SHADER, fragcode, "fragment shader");
    glLinkProgram(prog_id);
    int result;
    glGetProgramiv(prog_id, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        int len = 0;
        glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &len);
        auto infolog = std::make_unique<char[]>(len + 1);
        glGetProgramInfoLog(prog_id, len, &len, infolog.get());
        infolog[len] = '\0';
        error("OpenGL: program link error: {}\n", infolog.get());
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog_id;
}

void create_objects(unsigned &vao, unsigned &vbo, unsigned &ebo)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

} // namespace



namespace platform {

OpenGL::~OpenGL()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(context);
    SDL_Quit();
}

const int WIDTH = 256;
const int HEIGHT = 240;

void OpenGL::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error(fmt::format("can't initialize platform, SDL2 error: {}", SDL_GetError()));
    window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WIDTH, HEIGHT,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    prog_id = create_program();
    create_objects(vao, vbo, ebo);
    glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
    glUseProgram(prog_id);
    glUniform1i(glGetUniformLocation(prog_id, "tex"), 0);
    glm::mat4 projection = glm::ortho(0.0f, (float) WIDTH, (float) HEIGHT,  0.0f);
    glUniformMatrix4fv(glGetUniformLocation(prog_id, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void OpenGL::set_title(std::string_view title)
{
    SDL_SetWindowTitle(window, title.data());
}

void OpenGL::resize(std::size_t width, std::size_t height)
{
    glViewport(0, 0, width, height);
}

Texture OpenGL::create_texture(std::size_t width, std::size_t height)
{
    unsigned id;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    return (Texture) {
        .id = id, .width = width, .height = height,
    };
}

void OpenGL::update_texture(Texture &tex, const void *data)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void OpenGL::draw_texture(const Texture &tex, std::size_t x, std::size_t y)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glm::vec2 pos   = { (float) x, (float) y };
    glm::vec2 size  = { (float) tex.width, (float) tex.height };
    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3{pos, 0.0f});
    model = glm::translate(model, glm::vec3{0.5f * size.x, 0.5f * size.y, 0.0f});
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3{0.0f, 0.0f, 1.0f});
    model = glm::translate(model, glm::vec3{-0.5f * size.x, -0.5f * size.y, 0.0f});
    model = glm::scale(model, glm::vec3{size, 1.0f});
    int loc = glGetUniformLocation(prog_id, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

    glUseProgram(prog_id);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void OpenGL::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGL::swap()
{
    SDL_GL_SwapWindow(window);
}

void OpenGL::poll()
{
    for (SDL_Event ev; SDL_PollEvent(&ev); ) {
        switch (ev.type) {
        case SDL_QUIT:
            this->quit = true;
            break;
        case SDL_WINDOWEVENT:
            if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
                resize(ev.window.data1, ev.window.data2);
        }
    }
}

bool OpenGL::has_quit()
{
    return quit;
}

} // namespace platform

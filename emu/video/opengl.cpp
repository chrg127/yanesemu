#include <emu/video/opengl.hpp>

#include <exception>
#include <fmt/core.h>
#include <external/glad/glad.h>
#include <emu/util/debug.hpp>
#include <cstdlib>
#include <cstdio>
/*
#define STB_IMAGE_IMPLEMENTATION
#include <external/stb_image.h>
*/

static char vertcode[] = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    }
)";

static char fragcode[] = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;

    uniform sampler2D tex;

    void main()
    {
        FragColor = texture(tex, TexCoord);
        // FragColor = vec4(0.5f, 0.0f, 0.0f, 1.0f);
    }
)";

static float vertices[] = {
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // top right
     1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // bottom right
    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // top left
    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom left
};

unsigned indices[] = {
    0, 1, 2,
    1, 2, 3,
};

static GLenum glCheckError(const char *file, int line)
{
#ifdef DEBUG
    GLenum err;
    while (err = glGetError(), err != GL_NO_ERROR) {
        switch (err) {
        case GL_INVALID_ENUM:      error("opengl: invalid enum\n");      break;
        case GL_INVALID_VALUE:     error("opengl: invalid value\n");     break;
        case GL_INVALID_OPERATION: error("opengl: invalid operation\n"); break;
        case GL_OUT_OF_MEMORY:     error("opengl: out of memory\n");      break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error("opengl: invalid framebuffer operation\n");
            break;
        }
    }
    return err;
#else
    return GL_NO_ERROR;
#endif
}

static GLuint create_shader(GLuint progid, GLuint type, const char *code, const char *name)
{
    GLuint sid = glCreateShader(type);
    glShaderSource(sid, 1, &code, nullptr);
    glCompileShader(sid);
    GLint result = GL_FALSE;
    glGetShaderiv(sid, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        GLint len = 0;
        glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &len);
        auto infolog = std::make_unique<char[]>(len + 1);
        glGetShaderInfoLog(sid, len, &len, infolog.get());
        infolog[len] = '\0';
        error("OpenGL: {} compile error: {}\n", name, infolog.get());
        return 0;
    }
    glAttachShader(progid, sid);
    return sid;
}

static unsigned create_program()
{
    unsigned progid = glCreateProgram();
    auto vs = create_shader(progid, GL_VERTEX_SHADER, vertcode, "vertex shader");
    auto fs = create_shader(progid, GL_FRAGMENT_SHADER, fragcode, "fragment shader");
    glLinkProgram(progid);
    GLint result;
    glGetProgramiv(progid, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        GLint len = 0;
        glGetProgramiv(progid, GL_INFO_LOG_LENGTH, &len);
        auto infolog = std::make_unique<char[]>(len + 1);
        glGetProgramInfoLog(progid, len, &len, infolog.get());
        infolog[len] = '\0';
        error("OpenGL: program link error: {}\n", infolog.get());
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return progid;
}

static void create_objects(unsigned &vao, unsigned &vbo, unsigned &ebo)
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


namespace Video {

OpenGL::~OpenGL()
{
    if (!initialized)
        return;
    initialized = false;
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(context);
    SDL_Quit();
}

bool OpenGL::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        error("can't initialize video, SDL2 error: {}\n", SDL_GetError());
        return false;
    }
    window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              Context::DEF_WIDTH, Context::DEF_HEIGTH,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    context = SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        error("can't initialize video: GLAD error\n");
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    SDL_GL_SetSwapInterval(1);

    progid = create_program();
    create_objects(vao, vbo, ebo);
    glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
    glUseProgram(progid);
    glUniform1i(glGetUniformLocation(progid, "tex"), 0);

    initialized = true;
    return true;
}

void OpenGL::resize(int newwidth, int newheight)
{
    glViewport(0, 0, newwidth, newheight);
}

unsigned OpenGL::create_texture(std::size_t texw, std::size_t texh, unsigned char *data)
{
    unsigned id;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texw, texh, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    return id;
}

void OpenGL::update_texture(unsigned id, std::size_t texw, std::size_t texh, unsigned char *data)
{
    use_texture(id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texw, texh, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void OpenGL::use_texture(unsigned id)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
}

void OpenGL::draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(progid);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    SDL_GL_SwapWindow(window);
}

} // namespace Video

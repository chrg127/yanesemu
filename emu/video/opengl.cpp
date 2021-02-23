#include <emu/video/opengl.hpp>

#include <exception>
#include <fmt/core.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
        const char *errstr;
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

static void check_compile_error(unsigned id, GLenum type, const std::string &name)
{
#ifdef DEBUG
    int success;
    char infolog[512];
    auto printerr = [&](void (*getiv)(unsigned, GLenum, int *), void getinfo(unsigned, GLsizei, GLsizei *, char *))
    {
        getiv(id, type, &success);
        if (!success) {
            getinfo(id, 512, nullptr, infolog);
            throw std::runtime_error(fmt::format("error: {} compilation failed\nlog: {}\n", name, infolog));
        }
    };
    if (type == GL_COMPILE_STATUS) printerr(glGetShaderiv, glGetShaderInfoLog);
    else                           printerr(glGetProgramiv, glGetProgramInfoLog);
#endif
}

/*
static unsigned int load_texture_image(const char *pathname, GLenum index, GLenum colorformat, GLenum param, float border_color[3] = nullptr)
{
    unsigned int id;
    int width, height, channels;
    glActiveTexture(index);
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (param == GL_CLAMP_TO_BORDER) {
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
    }
    unsigned char *data = stbi_load(pathname, &width, &height, &channels, 0);
    assert(data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, colorformat, GL_UNSIGNED_BYTE, data);
    // glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return id;
}
*/

namespace Video {

OpenGL::~OpenGL()
{
    if (!initialized)
        return;
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool OpenGL::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        error("can't initialize video, SDL2 error: %s\n", SDL_GetError());
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    context = SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        error("can't initialize video: GLAD error\n");
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    SDL_GL_SetSwapInterval(1);

    create_shader();
    create_objects();
    glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
    glUseProgram(progid);
    glUniform1i(glGetUniformLocation(progid, "tex"), 0);

    initialized = true;
    return true;
}

Canvas OpenGL::create_canvas()
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
    Canvas res;
    res.tex_ids[0] = id;
    res.currid = 0;
    res.frame = new unsigned char[width*height*4]();
    return res;
}

void OpenGL::resize(int width, int height)
{

}

void OpenGL::update_screen(Canvas &canvas)
{
    static int pos = 0;
    for (int i = 0; i < 10; i++) {
        canvas.frame[pos] = 0xFF;
        canvas.frame[pos+1] = 0xFF;
        canvas.frame[pos+2] = 0xFF;
        canvas.frame[pos+3] = 0xFF;
        pos += 4;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, canvas.tex_ids[canvas.currid]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, canvas.frame);
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

void OpenGL::create_shader()
{
    unsigned vs, fs;

    auto compile = [](const char *code, unsigned &sid, GLenum type, const char *name)
    {
        sid = glCreateShader(type);
        glShaderSource(sid, 1, &code, nullptr);
        glCompileShader(sid);
        check_compile_error(sid, GL_COMPILE_STATUS, name);
    };

    vs = glCreateShader(GL_VERTEX_SHADER);
    compile(vertcode, vs, GL_VERTEX_SHADER, "vertex shader");
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    compile(fragcode, fs, GL_FRAGMENT_SHADER, "fragment shader");
    progid = glCreateProgram();
    glAttachShader(progid, vs);
    glAttachShader(progid, fs);
    glLinkProgram(progid);
    check_compile_error(progid, GL_LINK_STATUS, "program");
    glDeleteShader(vs);
    glDeleteShader(fs);
}

void OpenGL::create_objects()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

} // namespace Video

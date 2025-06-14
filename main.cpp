#include <iostream>
#include <vector>
#include <string>

#include <emscripten.h>
#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#include "glm/glm.hpp"

struct Shader
{
    const std::string vertexShader = 
    "#version 300 es \n"
    "layout(location=0) in vec3 position; \n"
    "void main() \n"
    "{"
    "   gl_Position = vec4(position.x, position.y, 0.0f, 1.0f); \n"
    "}";
    const std::string fragmentShader = 
    "#version 300 es \n"
    "precision mediump float; \n"
    "layout(location=0) out vec4 color; \n"
    "uniform float time; \n"
    "void main() \n"
    "{"
    "   color = vec4(0.0, sin(time), -sin(time), 1.0); \n"
    "}";
    unsigned int program;
};

void compile(Shader* shader)
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const char* vertexSrc = shader->vertexShader.c_str();
    const char* fragmentSrc = shader->fragmentShader.c_str();

    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);

    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    int vertexResult;
    int fragmentResult;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexResult);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentResult);

    if (vertexResult == GL_FALSE)
    {
        int vertexErrorLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &vertexErrorLength);
        char* vertexErrorMessage = reinterpret_cast<char*>(vertexErrorLength * sizeof(char));
        glGetShaderInfoLog(vertexShader, vertexErrorLength, &vertexErrorLength, vertexErrorMessage);
        std::cout << "Failed to compile vertex shader!" << std::endl;
        std::cout << vertexErrorMessage << std::endl;
        delete vertexErrorMessage;
    }

    if (fragmentResult == GL_FALSE)
    {
        int fragmentErrorLength;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &fragmentErrorLength);
        char* fragmentErrorMessage = reinterpret_cast<char*>(fragmentErrorLength * sizeof(char));
        glGetShaderInfoLog(fragmentShader, fragmentErrorLength, &fragmentErrorLength, fragmentErrorMessage);
        std::cout << "Failed to compile vertex shader!" << std::endl;
        std::cout << fragmentErrorMessage << std::endl;
        delete fragmentErrorMessage;
    }

    unsigned int program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    int linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        int linkErrorLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &linkErrorLength);
        char* linkErrorMessage = reinterpret_cast<char*>(linkErrorLength * sizeof(char));
        glGetProgramInfoLog(program, linkErrorLength, &linkErrorLength, linkErrorMessage);
        std::cout << "Failed to link program!" << std::endl;
        std::cout << linkErrorMessage << std::endl;
        delete linkErrorMessage;
    }

    shader->program = program;
}

struct Quad
{
    std::vector<float> vertices = {
        -0.5,  -0.5,
         0.0f,  0.5f,
         0.5f, -0.5f
    };
    unsigned int vbo;
};

void bind(Shader* shader)
{
    glUseProgram(shader->program);
}

void bind(Quad* quad, Shader* shader)
{
    glGenBuffers(1, &quad->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad->vbo);
    glBufferData(GL_ARRAY_BUFFER, quad->vertices.size() * sizeof(float), quad->vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(glGetAttribLocation(shader->program, "position"));
    glVertexAttribPointer(glGetAttribLocation(shader->program, "position"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
}

struct Globals
{
    const int WINDOW_WIDTH = 640;
    const int WINDOW_HEIGHT = 640;

    SDL_Window* window;
    SDL_GLContext context;

    SDL_Event event = {};
    bool quit = false;

    Shader shader;

    Quad quad;

    float time;
} glb;

void draw(Quad* quad, Shader* shader)
{
    glUniform1f(glGetUniformLocation(shader->program, "time"), glb.time);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void init()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    glb.window = SDL_CreateWindow(
        "SDL OpenGL Web", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED,
        glb.WINDOW_WIDTH,
        glb.WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );

    glb.context = SDL_GL_CreateContext(glb.window);

    glViewport(0, 0, glb.WINDOW_WIDTH, glb.WINDOW_HEIGHT);
    compile(&glb.shader);

    bind(&glb.quad, &glb.shader);
}

void cleanup()
{
    SDL_GL_DeleteContext(glb.context);
    SDL_DestroyWindow(glb.window);
    SDL_Quit();
}

void loop()
{
    while (SDL_PollEvent(&glb.event))
    {
        switch (glb.event.type)
        {
            case SDL_QUIT:
            {
                glb.quit = true;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    glb.time += 0.01;

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    bind(&glb.shader);

    draw(&glb.quad, &glb.shader);

    SDL_GL_SwapWindow(glb.window);

    if (glb.quit)
    {
        emscripten_cancel_main_loop();
        cleanup();
    }
}

int main()
{
    init();

    emscripten_set_main_loop(loop, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 0);

    return 0;
}
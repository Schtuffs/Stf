#include "Stf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "Types/ColourArrayList.h"
#include "Types/TriangleArrayList.h"
#include "Utils/Utils.h"

typedef struct WindowData {
    GLFWwindow* window;
    u64         width, height;
} WindowData;

static GLuint            shader    = 0;
static WindowData        window    = {0};
static TriangleArrayList triangles = {0};
static ColourArrayList   colours   = {0};

// ----- Helpers -----

void FramebufferCallback(GLFWwindow* w, int width, int height)
{
    (void)w;
    window.width  = width;
    window.height = height;
    glViewport(0, 0, width, height);
}

void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
    (void)w;
    (void)scancode;
    (void)action;
    (void)mods;
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(w, true);
    }
}

bool FileRead(const char* filename, char** out, int* size)
{
    // Data checks
    if (filename == NULL || out == NULL || size == NULL) {
        return false;
    }

    // Create file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    // Seek file end
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }

    // Reallocate
    if (*out) {
        free(*out);
    }
    *size = ftell(file);
    rewind(file);

    // Allocations
    *out = malloc(*size + 1);
    if (*out == NULL) {
        fclose(file);
        return false;
    }

    // Get data
    int read = fread(*out, 1, *size, file);
    if (read != *size) {
        fclose(file);
        return false;
    }

    (*out)[*size] = '\0';
    fclose(file);

    return true;
}

u32 CompileErrors(GLuint id, GLuint type)
{
    char    error[1024];
    GLsizei read;
    GLint   status = GL_TRUE;

    // For the vertex and fragment shaders
    glGetShaderiv(id, type, &status);
    if (status != GL_TRUE) {
        if (type == GL_COMPILE_STATUS) {
            glGetShaderInfoLog(id, 1024, &read, error);
        } else {
            glGetProgramInfoLog(id, 1024, &read, error);
        }

        fprintf(stderr, "COMPILE ERROR: %s\n", error);
        return 0;
    }

    return id;
}

GLuint ReadProgram(const char* file, GLuint shaderType)
{
    char* code = NULL;
    int   size;
    if (!FileRead(file, &code, &size)) {
        return 0;
    }

    // Compile shader
    GLuint shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, ((const char**)(&code)), NULL);
    glCompileShader(shaderID);
    return CompileErrors(shaderID, GL_COMPILE_STATUS);
}

GLuint CreateShader(const char* vertexFile, const char* fragmentFile)
{
    GLuint vertShader = ReadProgram(vertexFile, GL_VERTEX_SHADER);
    GLuint fragShader = ReadProgram(fragmentFile, GL_FRAGMENT_SHADER);

    // Bind the shaders to the shading program
    GLuint shader = 0;
    if (vertShader && fragShader) {
        shader = glCreateProgram();
        glAttachShader(shader, vertShader);
        glAttachShader(shader, fragShader);
        glLinkProgram(shader);
        shader = CompileErrors(shader, GL_LINK_STATUS);
    }

    if (vertShader) {
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDeleteShader(fragShader);
    }

    return shader;
}

// ----- Creation / Destruction

bool StfWindowInit(u64 width, u64 height, const char* title)
{
    // GLFW init
    if (!glfwInit()) {
        perror("ERROR: Failed to init glfw.");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Window init
    window.width  = width;
    window.height = height;
    window.window = glfwCreateWindow(window.width, window.height, title, NULL, NULL);
    if (!window.window) {
        perror("ERROR: Failure to init window.");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window.window);
    glfwSetFramebufferSizeCallback(window.window, FramebufferCallback);
    glfwSetKeyCallback(window.window, KeyCallback);
    glfwSwapInterval(0);

    // Glad init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        perror("ERROR: Failure to initialize GLAD");
        StfWindowClose();
        return false;
    }

    // Shader init
    shader = CreateShader("../Stf/lib/default.vert", "../Stf/lib/default.frag");
    if (shader == 0) {
        perror("ERROR: Failure to initialize shader.");
        StfWindowClose();
        return false;
    }

    // Buffer inits
    triangles = TriangleArrayListInit(0);
    if (!TriangleArrayListIsValid(triangles)) {
        perror("ERROR: Failure to initialize triangles.");
        StfWindowClose();
        return false;
    }

    colours = ColourArrayListInit(0);
    if (!ColourArrayListIsValid(colours)) {
        perror("ERROR: Failure to initialize colours.");
        StfWindowClose();
        return false;
    }

    return true;
}

bool StfWindowClose()
{
    TriangleArrayListDestroy(&triangles);

    if (shader != 0) {
        glDeleteProgram(shader);
    }

    glfwDestroyWindow(window.window);
    window.window = NULL;

    glfwTerminate();
    return true;
}

void StfBackground(Colour c)
{
    glClearColor(c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f);
}

// ----- Read -----

bool StfWindowShouldClose() { return glfwWindowShouldClose(window.window); }

u64 StfWindowWidth() { return window.width; }

u64 StfWindowHeight() { return window.height; }

// ----- Update -----

void StfBeginRender()
{
    glClear(GL_COLOR_BUFFER_BIT);
    TriangleArrayListClear(&triangles);
    ColourArrayListClear(&colours);
}

void StfEndRender()
{
    // Render them triangles
    for (u64 i = 0; i < triangles.count; i++) {
        Triangle triangle = triangles.data[i];
        Colour   colour   = colours.data[i];
        // clang-format off
        float    vertices[] = {
            triangle.p1.x, triangle.p1.y, triangle.p1.z, colour.r / 255.f, colour.g / 255.f, colour.b / 255.f, colour.a / 255.f,
            triangle.p2.x, triangle.p2.y, triangle.p2.z, colour.r / 255.f, colour.g / 255.f, colour.b / 255.f, colour.a / 255.f,
            triangle.p3.x, triangle.p3.y, triangle.p3.z, colour.r / 255.f, colour.g / 255.f, colour.b / 255.f, colour.a / 255.f,
        };
        // clang-format on

        u32 VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(0));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glUseProgram(shader);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    // End of pipeline
    glfwSwapBuffers(window.window);
    glfwPollEvents();
}

//

void StfRenderTriangle(Vec2 p1, Vec2 p2, Vec2 p3, Colour colour)
{

    // clang-format off
    double rx1 = Map(p1.x, 0., StfWindowWidth(),  -1., 1.);
    double ry1 = Map(p1.y, 0., StfWindowHeight(), -1., 1.);
    double rx2 = Map(p2.x, 0., StfWindowWidth(),  -1., 1.);
    double ry2 = Map(p2.y, 0., StfWindowHeight(), -1., 1.);
    double rx3 = Map(p3.x, 0., StfWindowWidth(),  -1., 1.);
    double ry3 = Map(p3.y, 0., StfWindowHeight(), -1., 1.);
    
    Triangle t;
    t.p1 = (Vec3){rx1, ry1, 0.f};
    t.p2 = (Vec3){rx2, ry2, 0.f};
    t.p3 = (Vec3){rx3, ry3, 0.f};
    // clang-format on

    TriangleArrayListAdd(&triangles, t);
    ColourArrayListAdd(&colours, colour);
}

void StfRenderRect(i64 x, i64 y, i64 width, i64 height, Colour colour)
{
    // clang-format off
    double rx = Map(x,      0., StfWindowWidth(),  -1., 1.);
    double ry = Map(y,      0., StfWindowHeight(), -1., 1.);
    double wi = Map(width,  0., StfWindowWidth(),   0., 2.);
    double hi = Map(height, 0., StfWindowHeight(),  0., 2.);

    Triangle t1, t2;
    t1.p1 = (Vec3){rx,      ry,      0.f};
    t1.p2 = (Vec3){rx + wi, ry,      0.f};
    t1.p3 = (Vec3){rx + wi, ry + hi, 0.f};

    t2.p1 = (Vec3){rx,      ry,      0.f};
    t2.p3 = (Vec3){rx + wi, ry + hi, 0.f};
    t2.p2 = (Vec3){rx,      ry + hi, 0.f};
    // clang-format on

    TriangleArrayListAdd(&triangles, t1);
    TriangleArrayListAdd(&triangles, t2);

    ColourArrayListAdd(&colours, colour);
    ColourArrayListAdd(&colours, colour);
}

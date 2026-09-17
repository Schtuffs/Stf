#include "Stf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "Types/ArrayList.h"
#include "Utils/Utils.h"

#if !defined(STF_VERTEX_BUFFER_MAX_SIZE)
#define STF_VERTEX_BUFFER_MAX_SIZE 8192
#endif

#define MAX_Z 500ull

typedef struct GlobalData {
    GLFWwindow* window;
    i32         width, height;
} GlobalData;

typedef struct VertexBuffer {
    float* vertices;  // Vertex pos  - Vec3 (shader-location 0)
    u8*    colours;   // Colour pos  - 4*u8 (shader-location 1)
    float* texCoords; // Texture pos - Vec2 (shader-location 2)
    u32*   indices;   // 6 indices per quad

    i32 count;  // Number of quads in buffer
    u32 vao;    // ID of VAO for binding
    u32 vbo[4]; // ID of VBO's for binding
} VertexBuffer;

typedef struct RenderData {
    u32*       shader;
    Texture2D* texture;
    i32        count;
} RenderData;

typedef struct RenderBuffer {
    i32           vertexSize;
    VertexBuffer* vertexBuffer;

    i32         renderSize;
    RenderData* renderBuffer;
} RenderBuffer;

static GlobalData   stfData      = {0};
static RenderBuffer renderer     = {0};
static Texture2D    whiteTexture = {0};

// ----- Helpers -----

void FramebufferCallback(GLFWwindow* w, int width, int height)
{
    (void)w;
    stfData.width  = width;
    stfData.height = height;
    glViewport(0, 0, width, height);
}

void KeyCallback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
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

bool StfWindowInit(i32 width, i32 height, const char* title)
{
    // GLFW init
    if (glfwInit() == GLFW_FALSE) {
        perror("ERROR: Failed to init glfw.");
        return false;
    }

    // Versioning
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Window init
    stfData.window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!stfData.window) {
        perror("ERROR: Failure to init stfData.");
        glfwTerminate();
        return false;
    }

    // Setup window settings
    glfwMakeContextCurrent(stfData.window);
    glfwSetFramebufferSizeCallback(stfData.window, FramebufferCallback);
    glfwSetKeyCallback(stfData.window, KeyCallback);
    glfwSwapInterval(0);

    // Fix sizing
    glfwGetWindowSize(stfData.window, &width, &height);
    stfData.width  = width;
    stfData.height = height;

    // Glad init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        perror("ERROR: Failure to initialize GLAD");
        StfWindowClose();
        return false;
    }

    glEnable(GL_DEPTH_FUNC);
    glDepthFunc(GL_LEQUAL);

    // Prepare vertex buffer
    renderer.vertexSize   = 1;
    renderer.vertexBuffer = malloc(sizeof(VertexBuffer) * renderer.vertexSize);
    if (!renderer.vertexBuffer) {
        perror("ERROR: Failure to initialize vertex buffer.");
        StfWindowClose();
        return false;
    }

    // OpenGL inits
    glGenVertexArrays(1, &renderer.vertexBuffer[0].vao);
    u32 vertexVboCount =
        sizeof(renderer.vertexBuffer[0].vbo) / sizeof(renderer.vertexBuffer[0].vbo[0]);
    glGenBuffers(vertexVboCount, renderer.vertexBuffer[0].vbo);
    glBindVertexArray(renderer.vertexBuffer[0].vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // Buffer inits
    renderer.vertexBuffer->vertices  = malloc(sizeof(float) * STF_VERTEX_BUFFER_MAX_SIZE * 12);
    renderer.vertexBuffer->colours   = malloc(sizeof(u8) * STF_VERTEX_BUFFER_MAX_SIZE * 16);
    renderer.vertexBuffer->texCoords = malloc(sizeof(float) * STF_VERTEX_BUFFER_MAX_SIZE * 8);
    renderer.vertexBuffer->indices   = malloc(sizeof(float) * STF_VERTEX_BUFFER_MAX_SIZE * 6);

    // Prepare render buffer
    renderer.renderSize   = 1;
    renderer.renderBuffer = malloc(sizeof(RenderBuffer) * renderer.renderSize);
    if (!renderer.renderBuffer) {
        perror("ERROR: Failure to initialize render buffer.");
        StfWindowClose();
        return false;
    }

    // Buffer inits
    renderer.renderBuffer->shader  = malloc(sizeof(u32) * STF_VERTEX_BUFFER_MAX_SIZE);
    renderer.renderBuffer->texture = malloc(sizeof(Texture2D) * STF_VERTEX_BUFFER_MAX_SIZE);

    // Shader init
    u32 shader = CreateShader("../Stf/lib/default.vert", "../Stf/lib/default.frag");
    if (shader == 0) {
        perror("ERROR: Failure to initialize shader.");
        StfWindowClose();
        return false;
    }
    renderer.renderBuffer[0].shader[0] = shader;

    // Default white texture
    glGenTextures(1, &whiteTexture.id);
    glBindTexture(GL_TEXTURE_2D, whiteTexture.id);
    u8 pixels[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    whiteTexture.width  = 1;
    whiteTexture.height = 1;
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

bool StfWindowClose()
{
    if (whiteTexture.id != 0) {
        glDeleteTextures(1, &whiteTexture.id);
        whiteTexture.id = 0;
    }

    // if (renderer.renderBuffer && renderer.renderBuffer->shader != 0) {
    //     glDeleteProgram(renderer.renderBuffer->shader);
    //     renderer.renderBuffer->shader = 0;
    //     free(renderer.renderBuffer);
    // }

    if (renderer.vertexBuffer && renderer.vertexBuffer->vao != 0) {
        glDeleteVertexArrays(1, &renderer.vertexBuffer->vao);
        renderer.vertexBuffer->vao = 0;
    }

    // if (renderer.vao) {
    //     glDeleteVertexArrays(1, &renderer.vao);
    //     renderer.vao = 0;
    // }

    if (stfData.window) {
        glfwDestroyWindow(stfData.window);
    }
    stfData.window = NULL;

    glfwTerminate();
    return true;
}

void StfBackground(Colour c) { glClearColor(c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f); }

// ----- Read -----

bool StfWindowShouldClose() { return glfwWindowShouldClose(stfData.window); }

i32 StfWindowWidth() { return stfData.width; }

i32 StfWindowHeight() { return stfData.height; }

Vec2 StfMousePos()
{
    double x, y;
    glfwGetCursorPos(stfData.window, &x, &y);
    return (Vec2){x, y};
}

// ----- Update -----

void StfBeginRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer.vertexBuffer->count = 0;
    renderer.renderBuffer->count = 0;
}

void StfEndRender()
{
    // Rendering to screen - loop through each buffer
    for (u64 vertBuf = 0; vertBuf < (u64)renderer.vertexSize; vertBuf++) {
        VertexBuffer* vb = &renderer.vertexBuffer[vertBuf];
        glBindVertexArray(vb->vao);

        glBindBuffer(GL_ARRAY_BUFFER, vb->vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vb->count * 12, vb->vertices,
                     GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));

        glBindBuffer(GL_ARRAY_BUFFER, vb->vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(u8) * vb->count * 16, vb->colours, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(u8), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, vb->vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vb->count * 8, vb->texCoords,
                     GL_DYNAMIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vb->vbo[3]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i32) * vb->count * 6, vb->indices,
                     GL_DYNAMIC_DRAW);

        glUseProgram(renderer.renderBuffer->shader[vertBuf]);
        glBindTexture(GL_TEXTURE_2D, renderer.renderBuffer->texture[vertBuf].id);
        glDrawElements(GL_TRIANGLES, vb->count * 6, GL_UNSIGNED_INT, (void*)0);
    }

    // End of pipeline
    glfwSwapBuffers(stfData.window);
    glfwPollEvents();
}

//

static void AddVertexes(VertexBuffer* vb, Vec3 p1, Vec3 p2, Vec3 p3, Vec3 p4)
{
    u64 idx = vb->count * 12;

    vb->vertices[idx + 0] = p1.x;
    vb->vertices[idx + 1] = p1.y;
    vb->vertices[idx + 2] = p1.z;

    vb->vertices[idx + 3] = p2.x;
    vb->vertices[idx + 4] = p2.y;
    vb->vertices[idx + 5] = p2.z;

    vb->vertices[idx + 6] = p3.x;
    vb->vertices[idx + 7] = p3.y;
    vb->vertices[idx + 8] = p3.z;

    vb->vertices[idx + 9]  = p4.x;
    vb->vertices[idx + 10] = p4.y;
    vb->vertices[idx + 11] = p4.z;
}

static void AddIndices(VertexBuffer* vb)
{
    u64 idx = vb->count * 6;
    u64 off = vb->count * 4;

    vb->indices[idx + 0] = 0 + off;
    vb->indices[idx + 1] = 1 + off;
    vb->indices[idx + 2] = 3 + off;
    vb->indices[idx + 3] = 1 + off;
    vb->indices[idx + 4] = 2 + off;
    vb->indices[idx + 5] = 3 + off;
}

static void AddColour(VertexBuffer* vb, Colour colour)
{
    u64 idx = vb->count * 16;

    vb->colours[idx + 0] = colour.r;
    vb->colours[idx + 1] = colour.g;
    vb->colours[idx + 2] = colour.b;
    vb->colours[idx + 3] = colour.a;

    vb->colours[idx + 4] = colour.r;
    vb->colours[idx + 5] = colour.g;
    vb->colours[idx + 6] = colour.b;
    vb->colours[idx + 7] = colour.a;

    vb->colours[idx + 8]  = colour.r;
    vb->colours[idx + 9]  = colour.g;
    vb->colours[idx + 10] = colour.b;
    vb->colours[idx + 11] = colour.a;

    vb->colours[idx + 12] = colour.r;
    vb->colours[idx + 13] = colour.g;
    vb->colours[idx + 14] = colour.b;
    vb->colours[idx + 15] = colour.a;
}

static void AddTexture2D(VertexBuffer* vb, Texture2D tex)
{
    u64 idx = vb->count * 8;

    vb->texCoords[idx + 0] = 0.f;
    vb->texCoords[idx + 1] = 0.f;

    vb->texCoords[idx + 2] = 0.f;
    vb->texCoords[idx + 3] = 1.f;

    vb->texCoords[idx + 4] = 1.f;
    vb->texCoords[idx + 5] = 1.f;

    vb->texCoords[idx + 6] = 1.f;
    vb->texCoords[idx + 7] = 0.f;

    renderer.renderBuffer->texture[renderer.renderBuffer->count] = tex;
}

static void AddShape(Vec3 v1, Vec3 v2, Vec3 v3, Vec3 v4, Colour colour, Texture2D tex)
{
    VertexBuffer* vb = &renderer.vertexBuffer[renderer.vertexSize - 1];

    AddVertexes(vb, v1, v2, v3, v4);
    AddIndices(vb);
    AddColour(vb, colour);
    AddTexture2D(vb, tex);

    vb->count++;
}

void StfRenderTriangle(Vec2 p1, Vec2 p2, Vec2 p3, Colour colour)
{
    // clang-format off
    double rx1 = Map(p1.x, 0., StfWindowWidth(),  -1.,  1.);
    double ry1 = Map(p1.y, 0., StfWindowHeight(),  1., -1.);
    double rx2 = Map(p2.x, 0., StfWindowWidth(),  -1.,  1.);
    double ry2 = Map(p2.y, 0., StfWindowHeight(),  1., -1.);
    double rx3 = Map(p3.x, 0., StfWindowWidth(),  -1.,  1.);
    double ry3 = Map(p3.y, 0., StfWindowHeight(),  1., -1.);
    // clang-format on

    Vec3 v1, v2, v3;

    v1.x = rx1;
    v1.y = ry1;
    v1.z = 0.f;

    v2.x = rx2;
    v2.y = ry2;
    v1.z = 0.f;

    v3.x = rx3;
    v3.y = ry3;
    v1.z = 0.f;

    AddShape(v1, v2, v1, v3, colour, whiteTexture);
}

void StfRenderRect(i64 x, i64 y, i64 width, i64 height, Colour colour)
{
    // clang-format off
    double rx = Map(x,      0., StfWindowWidth(),  -1.,  1.);
    double ry = Map(y,      0., StfWindowHeight(),  1., -1.);
    double wi = Map(width,  0., StfWindowWidth(),   0.,  2.);
    double hi = Map(height, 0., StfWindowHeight(),  0.,  2.);
    // clang-format on

    Vec3 v1, v2, v3, v4;

    v1.x = rx;
    v1.y = ry;
    v1.z = 0.;

    v2.x = rx;
    v2.y = ry - hi;
    v2.z = 0.;

    v3.x = rx + wi;
    v3.y = ry - hi;
    v3.z = 0.;

    v4.x = rx + wi;
    v4.y = ry;
    v4.z = 0.;

    AddShape(v1, v2, v3, v4, colour, whiteTexture);
}

void StfRenderTexture(Texture2D texture, i64 x, i64 y, Colour tint)
{
    // clang-format off
    double rx = Map(x,              0., StfWindowWidth(),  -1.,  1.);
    double ry = Map(y,              0., StfWindowHeight(),  1., -1.);
    double wi = Map(texture.width,  0., StfWindowWidth(),   0.,  2.);
    double hi = Map(texture.height, 0., StfWindowHeight(),  0.,  2.);
    // clang-format on

    Vec3 v1, v2, v3, v4;

    v1.x = rx;
    v1.y = ry;
    v1.z = 0.;

    v2.x = rx;
    v2.y = ry - hi;
    v2.z = 0.;

    v3.x = rx + wi;
    v3.y = ry - hi;
    v3.z = 0.;

    v4.x = rx + wi;
    v4.y = ry;
    v4.z = 0.;

    AddShape(v1, v2, v3, v4, tint, texture);
}

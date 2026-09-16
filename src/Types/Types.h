#pragma once

typedef struct Colour {
    u8 r, g, b, a;
} Colour;

// For compiler type safety
// clang-format off
#if defined(__STDC_VERSION__) && __STDC_VERSION >= 202311L

constexpr Colour WHITE   = {255, 255, 255, 255};
constexpr Colour BLACK   = {  0,   0,   0, 255};

constexpr Colour RED     = {255,   0,   0, 255};
constexpr Colour YELLOW  = {255, 255,   0, 255};
constexpr Colour GREEN   = {  0, 255,   0, 255};
constexpr Colour CYAN    = {  0, 255, 255, 255};
constexpr Colour BLUE    = {  0,   0, 255, 255};
constexpr Colour MAGENTA = {255,   0, 255, 255};

#else

#define WHITE   (Colour){255, 255, 255, 255}
#define BLACK   (Colour){  0,   0,   0, 255}

#define RED     (Colour){255,   0,   0, 255}
#define YELLOW  (Colour){255, 255,   0, 255}
#define GREEN   (Colour){  0, 255,   0, 255}
#define CYAN    (Colour){  0, 255, 255, 255}
#define BLUE    (Colour){  0,   0, 255, 255}
#define MAGENTA (Colour){255,   0, 255, 255}

#endif
// clang-format on

typedef struct Image {
    void* data;
    u64   width;
    u64   height;
} Image;

typedef struct Texture {
    u32 id;
    u64 width;
    u64 height;
} Texture;

typedef struct Vec2 {
    float x, y;
} Vec2;

typedef struct Vec3 {
    float x, y, z;
} Vec3;

typedef struct Vec4 {
    float x, y, z, w;
} Vec4;

typedef struct Mesh {
    int  vertCount;
    int* verts;
    int  vbo;
} Mesh;

typedef struct Triangle {
    Vec3 p1, p2, p3;
} Triangle;

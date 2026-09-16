#pragma once

#include "Types/ShortTypes.h"
#include "Types/Types.h"

#include "StfLoad.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define VERSION_MAJOR 0
#define VERSION_MINOR 2
#define VERSION "0.2"

// ----- Creation / Destruction -----

// Initializes the library.
bool StfWindowInit(i32 width, i32 height, const char* title);
// Frees memory created by library.
bool StfWindowClose();
// Sets the window background
void StfBackground(Colour colour);

// ----- Read -----

// Returns the width of the window.
i32 StfWindowWidth();
// Returns the height of the window.
i32 StfWindowHeight();
// Checks if the window should close or not.
bool StfWindowShouldClose();

// ----- Update -----

// Prepares system for rendering next frame.
void StfBeginRender();
// Renders the next frame.
void StfEndRender();

// Add triangle to the render pipeline.
void StfRenderTriangle(Vec2 p1, Vec2 p2, Vec2 p3, Colour colour);
// Adds a rectangle to the render pipeline.
void StfRenderRect(i64 x, i64 y, i64 width, i64 height, Colour colour);
// Renders a texture to the screen.
void StfRenderTexture(Texture texture, i64 x, i64 y, Colour tint);

#if defined(__cplusplus)
}
#endif

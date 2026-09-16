#pragma once

#include "Types/ShortTypes.h"
#include "Types/Types.h"

// ----- Creation / Destruction -----

// Loads an image from a file.
Image StfLoadImage(const char* filename);
// Loads a texture from an image.
Texture StfLoadTexture(Image image);

// ----- Read -----

// Checks if an image is valid.
bool StfIsValidImage(Image image);
// Checks if a texture is valid.
bool StfIsValidTexture(Texture tex);

// ----- Update -----

// Unloads an image.
void StfUnloadImage(Image image);
// Unloads a texture.
void StfUnloadTexture(Texture texture);

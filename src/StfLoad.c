#include "StfLoad.h"

#include <stdlib.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "stb_image.h"

// ----- Creation / Destruction -----

Image StfLoadImage(const char* filename)
{
    if (!filename) {
        return (Image){0};
    }

    Image image = {0};
    int   width, height, channels = 4;
    stbi_set_flip_vertically_on_load(true);
    image.data = stbi_load(filename, &width, &height, &channels, channels);
    if (!image.data) {
        return (Image){0};
    }

    image.width  = width;
    image.height = height;

    return image;
}

Texture2D StfLoadTexture(Image image)
{
    Texture2D tex = {0};
    if (!StfIsValidImage(image)) {
        return tex;
    }

    glGenTextures(1, &tex.id);
    if (!tex.id) {
        return tex;
    }

    tex.width  = image.width;
    tex.height = image.height;

    glBindTexture(GL_TEXTURE_2D, tex.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return tex;
}

// ----- Read -----

bool StfIsValidImage(Image image) { return (image.data && image.width > 0 && image.height > 0); }

bool StfIsValidTexture(Texture2D tex) { return (tex.id && tex.width > 0 && tex.height > 0); }

// ----- Update -----

void StfUnloadImage(Image image) { free(image.data); }

void StfUnloadTexture(Texture2D tex) { glDeleteTextures(1, &tex.id); }

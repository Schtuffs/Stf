#include "StfLoad.h"

#include <stdlib.h>

#include "stb_image.h"

// ----- Creation / Destruction -----

Image StfLoadImage(const char* filename)
{
    if (!filename) {
        return (Image){0};
    }

    Image image = {0};
    int   width, height, channels = 4;
    image.data = stbi_load(filename, &width, &height, &channels, channels);
    if (!image.data) {
        return (Image){0};
    }

    image.width  = width;
    image.height = height;

    return image;
}

// ----- Read -----

bool StfIsValidImage(Image image) { return (image.data && image.width > 0 && image.height > 0); }

// ----- Update -----

void StfUnloadImage(Image image) { free(image.data); }

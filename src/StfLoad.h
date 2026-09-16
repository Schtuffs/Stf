#pragma once

#include "Types/ShortTypes.h"
#include "Types/Types.h"

// ----- Creation / Destruction -----

Image StfLoadImage(const char* filename);

// ----- Read -----

bool StfIsValidImage(Image image);

// ----- Update -----

void StfUnloadImage(Image image);

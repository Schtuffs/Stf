#include "Utils/Utils.h"

double Map(double value, double currentMin, double currentMax, double newMin, double newMax)
{
    value -= currentMin;

    double currentRange = currentMax - currentMin;
    if (currentRange != 0) {
        value /= currentRange;
    }

    double newRange = newMax - newMin;
    value *= newRange;

    value += newMin;

    return value;
}

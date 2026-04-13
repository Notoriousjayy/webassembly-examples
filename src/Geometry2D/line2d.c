#include "line2d.h"

/*******************************************************************************
 * Line2D Operations
 ******************************************************************************/

float line2d_length(Line2D line) {
    return vec2_magnitude(vec2_sub(line.end, line.start));
}

float line2d_length_sq(Line2D line) {
    return vec2_magnitude_sq(vec2_sub(line.end, line.start));
}

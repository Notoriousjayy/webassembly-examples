#ifndef GEOMETRY2D_TYPES_H
#define GEOMETRY2D_TYPES_H

#include <stddef.h>
#include "../vectors.h"

typedef vec2 Point2D;

typedef struct Line2D {
    Point2D start;
    Point2D end;
} Line2D;

typedef struct Circle {
    Point2D position;
    float radius;
} Circle;

typedef struct Rectangle2D {
    Point2D origin;
    vec2 size;
} Rectangle2D;

typedef struct OrientedRectangle {
    Point2D position;
    vec2 half_extents;
    float rotation; /* degrees */
} OrientedRectangle;

typedef struct Interval2D {
    float min;
    float max;
} Interval2D;

typedef struct BoundingShape {
    Circle *circles;
    int num_circles;
    Rectangle2D *rectangles;
    int num_rectangles;
} BoundingShape;

/* Small constructors/defaults used by current sources */
static inline Line2D line2d_create(Point2D start, Point2D end) {
    return (Line2D){ .start = start, .end = end };
}

static inline Circle circle_create(Point2D position, float radius) {
    return (Circle){ .position = position, .radius = radius };
}

static inline Circle circle_default(void) {
    return (Circle){ .position = { .x = 0.0f, .y = 0.0f }, .radius = 0.0f };
}

static inline Rectangle2D rectangle2d_create(Point2D origin, vec2 size) {
    return (Rectangle2D){ .origin = origin, .size = size };
}

static inline Rectangle2D rectangle2d_default(void) {
    return (Rectangle2D){
        .origin = { .x = 0.0f, .y = 0.0f },
        .size = { .x = 0.0f, .y = 0.0f }
    };
}

#endif /* GEOMETRY2D_TYPES_H */

/**
 * @file geom3d_types.h
 * @brief Core 3D geometry type definitions and inline constructors
 */
#ifndef GEOM3D_TYPES_H
#define GEOM3D_TYPES_H

#include "vectors.h"
#include "matrices.h"

#include <stdbool.h>
#include <stdio.h>

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef vec3 Point3D;

typedef struct Line3D {
    Point3D start;
    Point3D end;
} Line3D;

typedef struct Ray3D {
    Point3D origin;
    vec3    direction;  /* Should be normalized */
} Ray3D;

typedef struct Sphere {
    Point3D position;
    float   radius;
} Sphere;

typedef struct AABB {
    Point3D position;   /* Center */
    vec3    size;       /* HALF SIZE */
} AABB;

typedef struct OBB {
    Point3D position;   /* Center */
    vec3    size;       /* HALF SIZE */
    mat3    orientation;
} OBB;

typedef struct Plane {
    vec3  normal;
    float distance;
} Plane;

typedef struct Triangle {
    union {
        struct {
            Point3D a;
            Point3D b;
            Point3D c;
        };
#ifndef NO_EXTRAS
        struct {
            Point3D p1;
            Point3D p2;
            Point3D p3;
        };
#endif
        Point3D points[3];
        float   values[9];
    };
} Triangle;

typedef struct Interval3D {
    float min;
    float max;
} Interval3D;

typedef struct Frustum {
    union {
        struct {
            Plane top;
            Plane bottom;
            Plane left;
            Plane right;
            Plane near_plane;   /* Renamed from _near to avoid reserved word issues */
            Plane far_plane;    /* Renamed from _far to avoid reserved word issues */
        };
        Plane planes[6];
    };
} Frustum;

typedef struct RaycastResult {
    vec3  point;
    vec3  normal;
    float t;
    bool  hit;
} RaycastResult;

typedef struct BVHNode {
    AABB            bounds;
    struct BVHNode* children;       /* Array of 8 children or NULL */
    int             num_triangles;
    int*            triangles;      /* Indices into mesh triangles */
} BVHNode;

typedef struct Mesh {
    int num_triangles;
    union {
        Triangle* triangles;
        Point3D*  vertices;
        float*    values;
    };
    BVHNode* accelerator;
} Mesh;

typedef struct Model {
    Mesh*         content;
    AABB          bounds;
    vec3          position;
    vec3          rotation;
    bool          flag;
    struct Model* parent;
} Model;

/* Dynamic array for contact points (replaces std::vector<vec3>) */
typedef struct ContactArray {
    vec3* data;
    int   count;
    int   capacity;
} ContactArray;

typedef struct CollisionManifold {
    bool         colliding;
    vec3         normal;
    float        depth;
    ContactArray contacts;
} CollisionManifold;

/* Dynamic array for Line3D (replaces std::vector<Line>) */
typedef struct Line3DArray {
    Line3D* data;
    int     count;
    int     capacity;
} Line3DArray;

/* Dynamic array for Plane (replaces std::vector<Plane>) */
typedef struct PlaneArray {
    Plane* data;
    int    count;
    int    capacity;
} PlaneArray;

/*******************************************************************************
 * Type Aliases (NO_EXTRAS compatibility)
 ******************************************************************************/

#ifndef NO_EXTRAS
typedef Line3D     Line;
typedef Ray3D      Ray;
typedef AABB       Rectangle3D;
typedef Interval3D Interval;
typedef Point3D    Point;
#endif

/*******************************************************************************
 * Constructors / Initializers
 ******************************************************************************/

static inline Line3D line3d_create(Point3D start, Point3D end) {
    return (Line3D){ .start = start, .end = end };
}

static inline Line3D line3d_default(void) {
    return (Line3D){ .start = {{{0, 0, 0}}}, .end = {{{0, 0, 0}}} };
}

static inline Ray3D ray3d_create(Point3D origin, vec3 direction) {
    return (Ray3D){ .origin = origin, .direction = vec3_normalized(direction) };
}

static inline Ray3D ray3d_default(void) {
    return (Ray3D){ .origin = {{{0, 0, 0}}}, .direction = {{{0, 0, 1}}} };
}

static inline void ray3d_normalize_direction(Ray3D* self) {
    self->direction = vec3_normalized(self->direction);
}

static inline Sphere sphere_create(Point3D position, float radius) {
    return (Sphere){ .position = position, .radius = radius };
}

static inline Sphere sphere_default(void) {
    return (Sphere){ .position = {{{0, 0, 0}}}, .radius = 1.0f };
}

static inline AABB aabb_create(Point3D position, vec3 size) {
    return (AABB){ .position = position, .size = size };
}

static inline AABB aabb_default(void) {
    return (AABB){ .position = {{{0, 0, 0}}}, .size = {{{1, 1, 1}}} };
}

static inline OBB obb_create(Point3D position, vec3 size, mat3 orientation) {
    return (OBB){ .position = position, .size = size, .orientation = orientation };
}

static inline OBB obb_create_simple(Point3D position, vec3 size) {
    return (OBB){ .position = position, .size = size, .orientation = mat3_identity() };
}

static inline OBB obb_default(void) {
    return (OBB){ .position = {{{0, 0, 0}}}, .size = {{{1, 1, 1}}}, .orientation = mat3_identity() };
}

static inline Plane plane_create(vec3 normal, float distance) {
    return (Plane){ .normal = normal, .distance = distance };
}

static inline Plane plane_default(void) {
    return (Plane){ .normal = {{{1, 0, 0}}}, .distance = 0.0f };
}

static inline Triangle triangle_create(Point3D a, Point3D b, Point3D c) {
    Triangle t;
    t.a = a;
    t.b = b;
    t.c = c;
    return t;
}

static inline Triangle triangle_default(void) {
    Triangle t = {0};
    return t;
}

static inline Frustum frustum_default(void) {
    Frustum f = {0};
    return f;
}

static inline BVHNode bvhnode_default(void) {
    return (BVHNode){
        .bounds = aabb_default(),
        .children = NULL,
        .num_triangles = 0,
        .triangles = NULL
    };
}

static inline Mesh mesh_default(void) {
    return (Mesh){
        .num_triangles = 0,
        .triangles = NULL,
        .accelerator = NULL
    };
}

static inline Model model_default(void) {
    return (Model){
        .content = NULL,
        .bounds = aabb_default(),
        .position = {{{0, 0, 0}}},
        .rotation = {{{0, 0, 0}}},
        .flag = false,
        .parent = NULL
    };
}

#endif /* GEOM3D_TYPES_H */

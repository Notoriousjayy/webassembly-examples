/* render.c - SDL3 + WebGL2 3D renderer for the WebAssembly target
 *
 * Drives a 3D scene using:
 *   - OrbitCamera with mouse/touch orbit + scroll zoom
 *   - MVP pipeline with perspective projection
 *   - Wireframe AABB, OBB, Sphere, and ground grid rendering
 *   - Frustum culling of scene objects
 *   - type_bridge.h for Cyclone ↔ game-math type conversions
 */

#include "render.h"
#include "camera.h"
#include "matrices.h"
#include "vectors.h"
#include "compare.h"
#include "type_bridge.h"
#include "Geometry3D/geom3d_types.h"
#include "Geometry3D/geom3d_primitives.h"
#include "Geometry3D/geom3d_frustum.h"

#ifndef __EMSCRIPTEN__
#error "This renderer is intended for Emscripten/WebAssembly builds only."
#endif

#include <SDL3/SDL.h>
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* Constants                                                                 */
/* ------------------------------------------------------------------------- */

#define MAX_SCENE_OBJECTS 16
#define SPHERE_RINGS      16
#define SPHERE_SEGMENTS   24
#define GRID_HALF_EXTENT  5
#define GRID_STEP         1

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------------- */
/* Scene object types                                                        */
/* ------------------------------------------------------------------------- */

typedef enum {
    SCENE_OBJ_AABB,
    SCENE_OBJ_OBB,
    SCENE_OBJ_SPHERE
} SceneObjType;

typedef struct SceneObject {
    SceneObjType type;
    union {
        AABB     aabb;
        OBB      obb;
        Sphere   sphere;
    };
    vec3  color;       /* wireframe color */
    float spin_speed;  /* radians per second around Y */
    float spin_angle;  /* accumulated rotation */
} SceneObject;

/* ------------------------------------------------------------------------- */
/* Global renderer state                                                     */
/* ------------------------------------------------------------------------- */

static SDL_Window   *g_window            = NULL;
static SDL_GLContext g_context            = NULL;
static bool          g_initialized       = false;
static bool          g_main_loop_started = false;
static bool          g_running           = false;

/* Shader program */
static GLuint g_program        = 0;
static GLint  g_loc_mvp        = -1;
static GLint  g_loc_color      = -1;

/* Dynamic line buffer (shared across all draws per frame) */
static GLuint  g_vao           = 0;
static GLuint  g_vbo           = 0;
static float  *g_line_buf      = NULL;
static int     g_line_buf_cap  = 0;  /* floats capacity */

/* Camera */
static OrbitCamera g_orbit_cam;

/* Scene */
static SceneObject g_objects[MAX_SCENE_OBJECTS];
static int         g_num_objects = 0;

/* Grid */
static GLuint  g_grid_vao     = 0;
static GLuint  g_grid_vbo     = 0;
static GLsizei g_grid_verts   = 0;

/* Input state */
static bool  g_mouse_down     = false;
static float g_mouse_prev_x   = 0.0f;
static float g_mouse_prev_y   = 0.0f;

/* Timing */
static double g_last_time     = 0.0;

/* Viewport */
static int g_viewport_w       = 1280;
static int g_viewport_h       = 720;

/* ------------------------------------------------------------------------- */
/* Shaders                                                                   */
/* ------------------------------------------------------------------------- */

static const char *VERT_SRC =
    "#version 300 es\n"
    "precision highp float;\n"
    "layout(location = 0) in vec3 aPos;\n"
    "uniform mat4 u_mvp;\n"
    "void main(void) {\n"
    "  gl_Position = u_mvp * vec4(aPos, 1.0);\n"
    "}\n";

static const char *FRAG_SRC =
    "#version 300 es\n"
    "precision mediump float;\n"
    "uniform vec3 u_color;\n"
    "out vec4 outColor;\n"
    "void main(void) {\n"
    "  outColor = vec4(u_color, 1.0);\n"
    "}\n";

/* ------------------------------------------------------------------------- */
/* GL helpers                                                                */
/* ------------------------------------------------------------------------- */

static GLuint compile_shader(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    if (shader == 0U) {
        fprintf(stderr, "glCreateShader failed\n");
        return 0U;
    }

    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint log_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char *log = (char *)malloc((size_t)log_len);
            if (log) {
                glGetShaderInfoLog(shader, log_len, NULL, log);
                fprintf(stderr, "Shader compilation error:\n%s\n", log);
                free(log);
            }
        }
        glDeleteShader(shader);
        return 0U;
    }
    return shader;
}

static GLuint link_program(GLuint vs, GLuint fs)
{
    GLuint program = glCreateProgram();
    if (program == 0U) {
        fprintf(stderr, "glCreateProgram failed\n");
        return 0U;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        GLint log_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char *log = (char *)malloc((size_t)log_len);
            if (log) {
                glGetProgramInfoLog(program, log_len, NULL, log);
                fprintf(stderr, "Program link error:\n%s\n", log);
                free(log);
            }
        }
        glDeleteProgram(program);
        return 0U;
    }
    return program;
}

/* ------------------------------------------------------------------------- */
/* Viewport                                                                  */
/* ------------------------------------------------------------------------- */

static void sync_viewport(void)
{
    int width = 0, height = 0;
    EMSCRIPTEN_RESULT rc =
        emscripten_get_canvas_element_size("#canvas", &width, &height);

    if (rc != EMSCRIPTEN_RESULT_SUCCESS) {
        width  = 1280;
        height = 720;
    }
    if (width  < 1) width  = 1;
    if (height < 1) height = 1;

    g_viewport_w = width;
    g_viewport_h = height;
    glViewport(0, 0, width, height);

    /* Update camera aspect ratio */
    camera_resize(&g_orbit_cam.base, width, height);
}

/* ========================================================================= */
/* Dynamic line-draw helpers                                                 */
/* ========================================================================= */

/**
 * Ensure the scratch line buffer has room for at least `need` additional
 * floats.  Returns a pointer to the start of the newly available region.
 */
static float *line_buf_grow(int need)
{
    int required = g_line_buf_cap + need;
    /* Round up to next 1024 */
    int new_cap = ((required + 1023) / 1024) * 1024;
    if (new_cap > g_line_buf_cap) {
        float *tmp = (float *)realloc(g_line_buf, (size_t)new_cap * sizeof(float));
        if (!tmp) return NULL;
        g_line_buf     = tmp;
        g_line_buf_cap = new_cap;
    }
    return g_line_buf;
}

/**
 * Upload `count` floats from g_line_buf into the shared VBO, draw as
 * GL_LINES, then logically clear the buffer.
 */
static void flush_lines(int float_count, mat4 mvp, vec3 color)
{
    if (float_count <= 0) return;

    float gl_mvp[16];
    mat4_fill_gl_array(&mvp, gl_mvp);

    glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, gl_mvp);
    glUniform3f(g_loc_color, color.x, color.y, color.z);

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)((size_t)float_count * sizeof(float)),
                 g_line_buf, GL_STREAM_DRAW);

    glDrawArrays(GL_LINES, 0, float_count / 3);
}

/* ========================================================================= */
/* Wireframe geometry generators                                             */
/* ========================================================================= */

/**
 * Append two endpoints (6 floats) of a line to the scratch buffer at `dst`.
 * Returns pointer advanced past the written floats.
 */
static float *emit_line(float *dst, vec3 a, vec3 b)
{
    *dst++ = a.x; *dst++ = a.y; *dst++ = a.z;
    *dst++ = b.x; *dst++ = b.y; *dst++ = b.z;
    return dst;
}

/**
 * Generate wireframe lines for an AABB and flush-draw them.
 * 12 edges × 2 endpoints × 3 floats = 72 floats.
 */
static void draw_aabb(AABB box, mat4 vp, vec3 color)
{
    vec3 mn = aabb_get_min(box);
    vec3 mx = aabb_get_max(box);

    /* 8 corners */
    vec3 c[8] = {
        vec3_make(mn.x, mn.y, mn.z),
        vec3_make(mx.x, mn.y, mn.z),
        vec3_make(mx.x, mx.y, mn.z),
        vec3_make(mn.x, mx.y, mn.z),
        vec3_make(mn.x, mn.y, mx.z),
        vec3_make(mx.x, mn.y, mx.z),
        vec3_make(mx.x, mx.y, mx.z),
        vec3_make(mn.x, mx.y, mx.z)
    };

    line_buf_grow(72);
    float *p = g_line_buf;

    /* Bottom face */
    p = emit_line(p, c[0], c[1]);
    p = emit_line(p, c[1], c[2]);
    p = emit_line(p, c[2], c[3]);
    p = emit_line(p, c[3], c[0]);
    /* Top face */
    p = emit_line(p, c[4], c[5]);
    p = emit_line(p, c[5], c[6]);
    p = emit_line(p, c[6], c[7]);
    p = emit_line(p, c[7], c[4]);
    /* Vertical edges */
    p = emit_line(p, c[0], c[4]);
    p = emit_line(p, c[1], c[5]);
    p = emit_line(p, c[2], c[6]);
    p = emit_line(p, c[3], c[7]);

    flush_lines(72, vp, color);
}

/**
 * Generate wireframe lines for an OBB and flush-draw them.
 * An OBB is an AABB transformed by its orientation matrix.
 */
static void draw_obb(OBB box, mat4 vp, vec3 color)
{
    /* Build local axes scaled by half-size */
    vec3 ax = vec3_mul_scalar(vec3_make(box.orientation._11,
                                        box.orientation._21,
                                        box.orientation._31), box.size.x);
    vec3 ay = vec3_mul_scalar(vec3_make(box.orientation._12,
                                        box.orientation._22,
                                        box.orientation._32), box.size.y);
    vec3 az = vec3_mul_scalar(vec3_make(box.orientation._13,
                                        box.orientation._23,
                                        box.orientation._33), box.size.z);

    vec3 center = box.position;

    /* 8 corners: ±ax ±ay ±az offset from center */
    vec3 c[8];
    for (int i = 0; i < 8; ++i) {
        float sx = (i & 1) ? 1.0f : -1.0f;
        float sy = (i & 2) ? 1.0f : -1.0f;
        float sz = (i & 4) ? 1.0f : -1.0f;

        c[i] = vec3_add(center,
                   vec3_add(vec3_mul_scalar(ax, sx),
                       vec3_add(vec3_mul_scalar(ay, sy),
                                vec3_mul_scalar(az, sz))));
    }

    line_buf_grow(72);
    float *p = g_line_buf;

    /* Edges: same topology as AABB
     * Bottom face (sz = -1): indices 0,1,2,3   (corners with bit2 = 0)
     * Top face    (sz = +1): indices 4,5,6,7 */
    p = emit_line(p, c[0], c[1]);
    p = emit_line(p, c[1], c[3]);
    p = emit_line(p, c[3], c[2]);
    p = emit_line(p, c[2], c[0]);
    p = emit_line(p, c[4], c[5]);
    p = emit_line(p, c[5], c[7]);
    p = emit_line(p, c[7], c[6]);
    p = emit_line(p, c[6], c[4]);
    p = emit_line(p, c[0], c[4]);
    p = emit_line(p, c[1], c[5]);
    p = emit_line(p, c[2], c[6]);
    p = emit_line(p, c[3], c[7]);

    flush_lines(72, vp, color);
}

/**
 * Generate wireframe lines for a sphere using latitude/longitude rings.
 */
static void draw_sphere(Sphere s, mat4 vp, vec3 color)
{
    /* Each ring/segment produces line segments; total lines =
     * SPHERE_RINGS * SPHERE_SEGMENTS + SPHERE_SEGMENTS * SPHERE_RINGS
     * We draw longitude arcs and latitude rings separately */

    const int total_lines = SPHERE_RINGS * SPHERE_SEGMENTS * 2;
    const int floats = total_lines * 6;
    line_buf_grow(floats);
    float *p = g_line_buf;

    /* Latitude rings */
    for (int ring = 1; ring < SPHERE_RINGS; ++ring) {
        float phi = (float)M_PI * (float)ring / (float)SPHERE_RINGS;
        float y   = s.radius * cosf(phi) + s.position.y;
        float r   = s.radius * sinf(phi);

        for (int seg = 0; seg < SPHERE_SEGMENTS; ++seg) {
            float theta0 = 2.0f * (float)M_PI * (float)seg       / (float)SPHERE_SEGMENTS;
            float theta1 = 2.0f * (float)M_PI * (float)(seg + 1) / (float)SPHERE_SEGMENTS;

            vec3 a = vec3_make(r * cosf(theta0) + s.position.x,
                               y,
                               r * sinf(theta0) + s.position.z);
            vec3 b = vec3_make(r * cosf(theta1) + s.position.x,
                               y,
                               r * sinf(theta1) + s.position.z);
            p = emit_line(p, a, b);
        }
    }

    /* Longitude arcs */
    for (int seg = 0; seg < SPHERE_SEGMENTS; ++seg) {
        float theta = 2.0f * (float)M_PI * (float)seg / (float)SPHERE_SEGMENTS;
        float ct = cosf(theta);
        float st = sinf(theta);

        for (int ring = 0; ring < SPHERE_RINGS; ++ring) {
            float phi0 = (float)M_PI * (float)ring       / (float)SPHERE_RINGS;
            float phi1 = (float)M_PI * (float)(ring + 1) / (float)SPHERE_RINGS;

            vec3 a = vec3_make(s.radius * sinf(phi0) * ct + s.position.x,
                               s.radius * cosf(phi0) + s.position.y,
                               s.radius * sinf(phi0) * st + s.position.z);
            vec3 b = vec3_make(s.radius * sinf(phi1) * ct + s.position.x,
                               s.radius * cosf(phi1) + s.position.y,
                               s.radius * sinf(phi1) * st + s.position.z);
            p = emit_line(p, a, b);
        }
    }

    int written = (int)(p - g_line_buf);
    flush_lines(written, vp, color);
}

/* ========================================================================= */
/* Grid                                                                      */
/* ========================================================================= */

static void build_grid(void)
{
    /* Lines along X and Z at y=0, from -GRID_HALF_EXTENT to +GRID_HALF_EXTENT */
    int lines_per_axis = (2 * GRID_HALF_EXTENT / GRID_STEP) + 1;
    int total_lines    = lines_per_axis * 2;  /* X lines + Z lines */
    int total_floats   = total_lines * 2 * 3; /* 2 endpoints × 3 floats */

    float *buf = (float *)malloc((size_t)total_floats * sizeof(float));
    if (!buf) return;

    float *p = buf;
    float lo = -(float)GRID_HALF_EXTENT;
    float hi =  (float)GRID_HALF_EXTENT;

    /* Lines parallel to Z axis */
    for (int i = -GRID_HALF_EXTENT; i <= GRID_HALF_EXTENT; i += GRID_STEP) {
        float x = (float)i;
        *p++ = x; *p++ = 0.0f; *p++ = lo;
        *p++ = x; *p++ = 0.0f; *p++ = hi;
    }

    /* Lines parallel to X axis */
    for (int i = -GRID_HALF_EXTENT; i <= GRID_HALF_EXTENT; i += GRID_STEP) {
        float z = (float)i;
        *p++ = lo; *p++ = 0.0f; *p++ = z;
        *p++ = hi; *p++ = 0.0f; *p++ = z;
    }

    g_grid_verts = (GLsizei)((p - buf) / 3);

    glGenVertexArrays(1, &g_grid_vao);
    glGenBuffers(1, &g_grid_vbo);

    glBindVertexArray(g_grid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_grid_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)((size_t)(p - buf) * sizeof(float)),
                 buf, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * (GLsizei)sizeof(float), NULL);

    free(buf);
}

static void draw_grid(mat4 vp)
{
    if (g_grid_vao == 0 || g_grid_verts == 0) return;

    float gl_mvp[16];
    mat4_fill_gl_array(&vp, gl_mvp);

    glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, gl_mvp);
    glUniform3f(g_loc_color, 0.25f, 0.25f, 0.25f);

    glBindVertexArray(g_grid_vao);
    glDrawArrays(GL_LINES, 0, g_grid_verts);
}

/* ========================================================================= */
/* Frustum culling                                                           */
/* ========================================================================= */

static bool cull_object(const SceneObject *obj, Frustum frustum)
{
    switch (obj->type) {
        case SCENE_OBJ_AABB:
            return !frustum_intersects_aabb(frustum, obj->aabb);
        case SCENE_OBJ_OBB:
            return !frustum_intersects_obb(frustum, obj->obb);
        case SCENE_OBJ_SPHERE:
            return !frustum_intersects_sphere(frustum, obj->sphere);
    }
    return false;
}

/* ========================================================================= */
/* Scene setup                                                               */
/* ========================================================================= */

static void build_scene(void)
{
    g_num_objects = 0;

    /* Object 0: AABB (cyan) at origin */
    g_objects[g_num_objects] = (SceneObject){
        .type       = SCENE_OBJ_AABB,
        .aabb       = aabb_create(vec3_make(0.0f, 0.75f, 0.0f),
                                  vec3_make(0.5f, 0.5f, 0.5f)),
        .color      = vec3_make(0.0f, 0.9f, 0.9f),
        .spin_speed = 0.0f,
        .spin_angle = 0.0f
    };
    g_num_objects++;

    /* Object 1: Sphere (green) to the right */
    g_objects[g_num_objects] = (SceneObject){
        .type       = SCENE_OBJ_SPHERE,
        .sphere     = sphere_create(vec3_make(2.5f, 1.0f, 0.0f), 0.7f),
        .color      = vec3_make(0.2f, 0.9f, 0.3f),
        .spin_speed = 0.0f,
        .spin_angle = 0.0f
    };
    g_num_objects++;

    /* Object 2: OBB (yellow) to the left, spinning */
    g_objects[g_num_objects] = (SceneObject){
        .type       = SCENE_OBJ_OBB,
        .obb        = obb_create_simple(vec3_make(-2.5f, 0.75f, 0.0f),
                                        vec3_make(0.4f, 0.6f, 0.3f)),
        .color      = vec3_make(0.95f, 0.85f, 0.15f),
        .spin_speed = 0.8f,
        .spin_angle = 0.0f
    };
    g_num_objects++;

    /* Object 3: AABB (magenta) behind */
    g_objects[g_num_objects] = (SceneObject){
        .type       = SCENE_OBJ_AABB,
        .aabb       = aabb_create(vec3_make(0.0f, 0.5f, -3.0f),
                                  vec3_make(0.8f, 0.3f, 0.8f)),
        .color      = vec3_make(0.9f, 0.2f, 0.7f),
        .spin_speed = 0.0f,
        .spin_angle = 0.0f
    };
    g_num_objects++;

    /* Object 4: Sphere (orange) in front */
    g_objects[g_num_objects] = (SceneObject){
        .type       = SCENE_OBJ_SPHERE,
        .sphere     = sphere_create(vec3_make(-1.0f, 0.5f, 2.5f), 0.5f),
        .color      = vec3_make(1.0f, 0.55f, 0.1f),
        .spin_speed = 0.0f,
        .spin_angle = 0.0f
    };
    g_num_objects++;

    /* Object 5: OBB (red) spinning faster */
    g_objects[g_num_objects] = (SceneObject){
        .type       = SCENE_OBJ_OBB,
        .obb        = obb_create_simple(vec3_make(1.5f, 1.2f, -1.5f),
                                        vec3_make(0.35f, 0.35f, 0.35f)),
        .color      = vec3_make(1.0f, 0.15f, 0.15f),
        .spin_speed = 1.5f,
        .spin_angle = 0.0f
    };
    g_num_objects++;
}

/* ========================================================================= */
/* Camera setup                                                              */
/* ========================================================================= */

static void init_camera(void)
{
    g_orbit_cam = orbit_camera_create_with_target(
        vec3_make(0.0f, 0.5f, 0.0f),   /* look at */
        6.0f                            /* distance */
    );

    /* Set initial orientation */
    orbit_camera_set_rotation(&g_orbit_cam, vec2_make(45.0f, 25.0f));

    /* Configure perspective on the base camera */
    float aspect = (float)g_viewport_w / (float)g_viewport_h;
    camera_set_perspective(&g_orbit_cam.base, 60.0f, aspect, 0.1f, 100.0f);

    /* Configure orbit limits */
    g_orbit_cam.zoom_distance_limit = vec2_make(2.0f, 25.0f);
    g_orbit_cam.y_rotation_limit    = vec2_make(-85.0f, 85.0f);
    g_orbit_cam.rotation_speed      = vec2_make(0.3f, 0.3f);
    g_orbit_cam.zoom_speed          = 2.0f;
}

/* ========================================================================= */
/* Input handling                                                            */
/* ========================================================================= */

static void process_events(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                g_running = false;
                emscripten_cancel_main_loop();
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                sync_viewport();
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    g_mouse_down   = true;
                    g_mouse_prev_x = event.button.x;
                    g_mouse_prev_y = event.button.y;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    g_mouse_down = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (g_mouse_down) {
                    float dx = event.motion.x - g_mouse_prev_x;
                    float dy = event.motion.y - g_mouse_prev_y;
                    g_mouse_prev_x = event.motion.x;
                    g_mouse_prev_y = event.motion.y;

                    /* Apply orbit rotation */
                    vec2 rot = orbit_camera_get_rotation(&g_orbit_cam);
                    rot.x += dx * g_orbit_cam.rotation_speed.x;
                    rot.y += dy * g_orbit_cam.rotation_speed.y;
                    orbit_camera_set_rotation(&g_orbit_cam, rot);
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                {
                    float zoom = orbit_camera_get_zoom(&g_orbit_cam);
                    zoom -= event.wheel.y * g_orbit_cam.zoom_speed;
                    orbit_camera_set_zoom(&g_orbit_cam, zoom);
                }
                break;

            default:
                break;
        }
    }
}

/* ========================================================================= */
/* Shutdown                                                                  */
/* ========================================================================= */

static void renderer_shutdown(void)
{
    if (g_vbo != 0U) { glDeleteBuffers(1, &g_vbo);         g_vbo = 0U; }
    if (g_vao != 0U) { glDeleteVertexArrays(1, &g_vao);    g_vao = 0U; }
    if (g_grid_vbo != 0U) { glDeleteBuffers(1, &g_grid_vbo);      g_grid_vbo = 0U; }
    if (g_grid_vao != 0U) { glDeleteVertexArrays(1, &g_grid_vao); g_grid_vao = 0U; }
    if (g_program != 0U) { glDeleteProgram(g_program); g_program = 0U; }

    free(g_line_buf);
    g_line_buf     = NULL;
    g_line_buf_cap = 0;

    if (g_context && g_window) {
        SDL_GL_DestroyContext(g_context);
        g_context = NULL;
    }
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }

    SDL_Quit();

    g_initialized       = false;
    g_main_loop_started = false;
    g_running           = false;
}

/* ========================================================================= */
/* Frame tick                                                                */
/* ========================================================================= */

static void tick(void)
{
    if (!g_running || !g_initialized) return;

    process_events();
    if (!g_running) return;

    sync_viewport();

    /* ----- Delta time ----- */
    double now = emscripten_get_now() / 1000.0;  /* seconds */
    float dt = (g_last_time > 0.0) ? (float)(now - g_last_time) : (1.0f / 60.0f);
    if (dt > 0.1f) dt = 0.1f;  /* clamp large spikes */
    g_last_time = now;

    /* ----- Update spinning OBBs ----- */
    for (int i = 0; i < g_num_objects; ++i) {
        if (g_objects[i].type == SCENE_OBJ_OBB && g_objects[i].spin_speed != 0.0f) {
            g_objects[i].spin_angle += g_objects[i].spin_speed * dt;
            mat3 rot = Rotation3x3(0.0f,
                                   RAD2DEG(g_objects[i].spin_angle),
                                   0.0f);
            g_objects[i].obb.orientation = rot;
        }
    }

    /* ----- Update camera ----- */
    orbit_camera_update(&g_orbit_cam, dt);

    /* ----- Build VP matrix ----- */
    mat4 view = camera_get_view_matrix(&g_orbit_cam.base);
    mat4 proj = camera_get_projection_matrix(&g_orbit_cam.base);
    mat4 vp   = mat4_mul(view, proj);

    /* ----- Frustum for culling ----- */
    Frustum frustum = camera_get_frustum(&g_orbit_cam.base);

    /* ----- Clear ----- */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(g_program);

    /* ----- Draw ground grid (never culled) ----- */
    draw_grid(vp);

    /* ----- Draw axis indicator at origin ----- */
    {
        line_buf_grow(18);
        float *p = g_line_buf;
        /* X axis - red */
        p = emit_line(p, vec3_make(0,0,0), vec3_make(1,0,0));
        flush_lines(6, vp, vec3_make(1.0f, 0.2f, 0.2f));

        p = g_line_buf;
        p = emit_line(p, vec3_make(0,0,0), vec3_make(0,1,0));
        flush_lines(6, vp, vec3_make(0.2f, 1.0f, 0.2f));

        p = g_line_buf;
        p = emit_line(p, vec3_make(0,0,0), vec3_make(0,0,1));
        flush_lines(6, vp, vec3_make(0.3f, 0.3f, 1.0f));
    }

    /* ----- Draw scene objects with frustum culling ----- */
    int culled = 0;
    for (int i = 0; i < g_num_objects; ++i) {
        if (cull_object(&g_objects[i], frustum)) {
            culled++;
            continue;
        }

        switch (g_objects[i].type) {
            case SCENE_OBJ_AABB:
                draw_aabb(g_objects[i].aabb, vp, g_objects[i].color);
                break;
            case SCENE_OBJ_OBB:
                draw_obb(g_objects[i].obb, vp, g_objects[i].color);
                break;
            case SCENE_OBJ_SPHERE:
                draw_sphere(g_objects[i].sphere, vp, g_objects[i].color);
                break;
        }
    }

    /* ----- Type bridge demonstration ----- */
    /* Show that Cyclone physics state can feed the render pipeline.
     * Here we convert the camera position to cyclone_Vector3 and back
     * to verify the bridge at runtime (diagnostic only). */
    {
        static int frame_counter = 0;
        frame_counter++;
        if ((frame_counter % 180) == 0) {
            vec3 cam_pos = camera_get_position(&g_orbit_cam.base);
            cyclone_Vector3 cy_pos = vec3_to_cyclone(cam_pos);
            vec3 roundtrip = vec3_from_cyclone(cy_pos);

            printf("[tick] frame=%d cam=(%.2f,%.2f,%.2f) "
                   "cyclone_rt=(%.2f,%.2f,%.2f) culled=%d/%d\n",
                   frame_counter,
                   cam_pos.x, cam_pos.y, cam_pos.z,
                   roundtrip.x, roundtrip.y, roundtrip.z,
                   culled, g_num_objects);
        }
    }

    SDL_GL_SwapWindow(g_window);
}

/* ========================================================================= */
/* Public API                                                                */
/* ========================================================================= */

EMSCRIPTEN_KEEPALIVE
int initWebGL(void)
{
    if (g_initialized) return 1;

    /* ----- SDL init ----- */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    g_window = SDL_CreateWindow(
        "testProject — 3D Scene",
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    g_context = SDL_GL_CreateContext(g_window);
    if (!g_context) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        g_window = NULL;
        SDL_Quit();
        return 0;
    }

    if (!SDL_GL_MakeCurrent(g_window, g_context)) {
        fprintf(stderr, "SDL_GL_MakeCurrent failed: %s\n", SDL_GetError());
        renderer_shutdown();
        return 0;
    }

    SDL_GL_SetSwapInterval(1);

    /* ----- Shaders ----- */
    GLuint vs = compile_shader(GL_VERTEX_SHADER, VERT_SRC);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, FRAG_SRC);
    if (vs == 0U || fs == 0U) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        renderer_shutdown();
        return 0;
    }

    g_program = link_program(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (g_program == 0U) {
        renderer_shutdown();
        return 0;
    }

    g_loc_mvp   = glGetUniformLocation(g_program, "u_mvp");
    g_loc_color = glGetUniformLocation(g_program, "u_color");

    /* ----- Dynamic line buffer VAO/VBO ----- */
    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * (GLsizei)sizeof(float), NULL);

    /* ----- Grid ----- */
    build_grid();

    /* ----- Scene ----- */
    build_scene();

    /* ----- Camera ----- */
    sync_viewport();
    init_camera();

    /* ----- GL state ----- */
    glClearColor(0.06f, 0.06f, 0.10f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glLineWidth(1.5f);

    printf("[initWebGL] SDL3 + WebGL2 3D renderer initialized\n");
    printf("[initWebGL] OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("[initWebGL] GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("[initWebGL] Scene objects: %d\n", g_num_objects);
    printf("[initWebGL] Frustum culling: enabled\n");
    printf("[initWebGL] Type bridge: active (vec3 <-> cyclone_Vector3, mat4 <-> cyclone_Matrix4)\n");

    g_initialized = true;
    return 1;
}

EMSCRIPTEN_KEEPALIVE
void startMainLoop(void)
{
    if (!g_initialized) {
        fprintf(stderr, "startMainLoop called before initWebGL\n");
        return;
    }

    if (g_main_loop_started) {
        printf("[startMainLoop] main loop already started, ignoring\n");
        return;
    }

    g_running           = true;
    g_main_loop_started = true;
    g_last_time         = emscripten_get_now() / 1000.0;

    printf("[startMainLoop] entering 3D main loop\n");
    emscripten_set_main_loop(tick, 0, 1);
}
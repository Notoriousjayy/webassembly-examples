/* render.c - SDL3 + WebGL2 renderer for the WebAssembly target */

#include "render.h"
#include "polygon.h"

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

/* ------------------------------------------------------------------------- */
/* Global renderer state                                                     */
/* ------------------------------------------------------------------------- */

static SDL_Window   *g_window              = NULL;
static SDL_GLContext g_context             = NULL;
static bool          g_initialized         = false;
static bool          g_main_loop_started   = false;
static bool          g_running             = false;

static GLuint   g_program          = 0;
static GLuint   g_vao              = 0;
static GLuint   g_vbo              = 0;
static GLsizei  g_vertex_count     = 0;

static Polygon  g_polygon;
static Point2D *g_base_verts       = NULL;
static float   *g_vertex_data      = NULL;
static size_t   g_vertex_float_cnt = 0U;

/* ------------------------------------------------------------------------- */
/* Shaders                                                                   */
/* ------------------------------------------------------------------------- */

static const char *VERT_SRC =
    "#version 300 es\n"
    "precision highp float;\n"
    "layout(location = 0) in vec2 aPos;\n"
    "void main(void) {\n"
    "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const char *FRAG_SRC =
    "#version 300 es\n"
    "precision mediump float;\n"
    "out vec4 outColor;\n"
    "void main(void) {\n"
    "  outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

/* ------------------------------------------------------------------------- */
/* Helpers                                                                   */
/* ------------------------------------------------------------------------- */

static void renderer_reset_heap_buffers(void)
{
    free(g_base_verts);
    g_base_verts = NULL;

    free(g_vertex_data);
    g_vertex_data = NULL;

    g_vertex_float_cnt = 0U;
    g_vertex_count = 0;
}

static void renderer_destroy_gl_objects(void)
{
    if (g_vbo != 0U) {
        glDeleteBuffers(1, &g_vbo);
        g_vbo = 0U;
    }

    if (g_vao != 0U) {
        glDeleteVertexArrays(1, &g_vao);
        g_vao = 0U;
    }

    if (g_program != 0U) {
        glDeleteProgram(g_program);
        g_program = 0U;
    }
}

static void renderer_shutdown(void)
{
    renderer_destroy_gl_objects();
    renderer_reset_heap_buffers();
    polygon_clear(&g_polygon);

    if (g_context != NULL && g_window != NULL) {
        SDL_GL_DestroyContext(g_context);
        g_context = NULL;
    }

    if (g_window != NULL) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }

    SDL_Quit();

    g_initialized = false;
    g_main_loop_started = false;
    g_running = false;
}

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
            if (log != NULL) {
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
            if (log != NULL) {
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

static void sync_viewport(void)
{
    int width = 0;
    int height = 0;

    const EMSCRIPTEN_RESULT rc =
        emscripten_get_canvas_element_size("#canvas", &width, &height);

    if (rc != EMSCRIPTEN_RESULT_SUCCESS) {
        width = 1280;
        height = 720;
    }

    if (width < 1) {
        width = 1;
    }
    if (height < 1) {
        height = 1;
    }

    glViewport(0, 0, width, height);
}

static bool init_geometry(void)
{
    polygon_init(&g_polygon);

    if (!polygon_make_regular_ngon(&g_polygon, 6U, 0.3)) {
        fprintf(stderr, "polygon_make_regular_ngon failed\n");
        return false;
    }

    if (!polygon_is_valid(&g_polygon)) {
        fprintf(stderr, "polygon is not valid\n");
        return false;
    }

    g_vertex_count = (GLsizei)g_polygon.count;
    g_vertex_float_cnt = (size_t)g_vertex_count * 2U;

    g_base_verts = (Point2D *)malloc(sizeof(Point2D) * g_polygon.count);
    if (g_base_verts == NULL) {
        fprintf(stderr, "malloc failed for g_base_verts\n");
        return false;
    }

    g_vertex_data = (float *)malloc(sizeof(float) * g_vertex_float_cnt);
    if (g_vertex_data == NULL) {
        fprintf(stderr, "malloc failed for g_vertex_data\n");
        return false;
    }

    for (size_t i = 0; i < g_polygon.count; ++i) {
        g_base_verts[i] = g_polygon.vertices[i];
        g_vertex_data[2U * i + 0U] = (float)g_polygon.vertices[i].x;
        g_vertex_data[2U * i + 1U] = (float)g_polygon.vertices[i].y;
    }

    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(sizeof(float) * g_vertex_float_cnt),
                 g_vertex_data,
                 GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          (GLsizei)(2 * (GLint)sizeof(float)),
                          (const void *)0);

    return true;
}

static void process_events(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                g_running = false;
                emscripten_cancel_main_loop();
                renderer_shutdown();
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                sync_viewport();
                break;

            default:
                break;
        }
    }
}

static void tick(void)
{
    static int frame_count = 0;

    if (!g_running) {
        return;
    }

    process_events();

    if (!g_running || !g_initialized) {
        return;
    }

    sync_viewport();

    ++frame_count;

    if (polygon_is_valid(&g_polygon) &&
        g_base_verts != NULL &&
        g_vertex_data != NULL &&
        g_vertex_float_cnt == (size_t)g_vertex_count * 2U)
    {
        const double t = (double)frame_count;
        const double angle = 0.05 * t;
        const double orbit_r = 0.6;
        const double tx = orbit_r * cos(0.01 * t);
        const double ty = orbit_r * sin(0.013 * t);
        const double c = cos(angle);
        const double s = sin(angle);

        for (size_t i = 0; i < (size_t)g_vertex_count; ++i) {
            const double x0 = g_base_verts[i].x;
            const double y0 = g_base_verts[i].y;

            const double xr = c * x0 - s * y0;
            const double yr = s * x0 + c * y0;

            const double x = xr + tx;
            const double y = yr + ty;

            g_polygon.vertices[i].x = x;
            g_polygon.vertices[i].y = y;

            g_vertex_data[2U * i + 0U] = (float)x;
            g_vertex_data[2U * i + 1U] = (float)y;
        }

        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        (GLsizeiptr)(sizeof(float) * g_vertex_float_cnt),
                        g_vertex_data);

        if ((frame_count % 30) == 0) {
            const Point2D *p0 = &g_polygon.vertices[0];
            const double per = polygon_perimeter(&g_polygon);
            printf("[tick] frame=%d first=(%.3f, %.3f) perimeter=%.3f\n",
                   frame_count, p0->x, p0->y, per);
        }
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (g_program != 0U && g_vao != 0U && g_vertex_count > 0) {
        glUseProgram(g_program);
        glBindVertexArray(g_vao);
        glDrawArrays(GL_LINE_LOOP, 0, g_vertex_count);
    }

    SDL_GL_SwapWindow(g_window);
}

EMSCRIPTEN_KEEPALIVE
int initWebGL(void)
{
    if (g_initialized) {
        return 1;
    }

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
        "testProject",
        1280,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (g_window == NULL) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    g_context = SDL_GL_CreateContext(g_window);
    if (g_context == NULL) {
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

    GLuint vs = compile_shader(GL_VERTEX_SHADER, VERT_SRC);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, FRAG_SRC);

    if (vs == 0U || fs == 0U) {
        if (vs != 0U) {
            glDeleteShader(vs);
        }
        if (fs != 0U) {
            glDeleteShader(fs);
        }
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

    if (!init_geometry()) {
        renderer_shutdown();
        return 0;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    sync_viewport();

    printf("[initWebGL] SDL3 + WebGL2 renderer initialized\n");
    printf("[initWebGL] OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("[initWebGL] GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

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

    g_running = true;
    g_main_loop_started = true;

    printf("[startMainLoop] entering main loop\n");
    emscripten_set_main_loop(tick, 0, 1);
}

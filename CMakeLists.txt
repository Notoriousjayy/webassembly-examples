cmake_minimum_required(VERSION 3.20)
project(testProject LANGUAGES C)

option(BUILD_WASM_HTML "Copy the static HTML/JS browser loader into the build directory" ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(SERVE_PORT "8000" CACHE STRING "Dev server port")
set(SERVE_HOST "0.0.0.0" CACHE STRING "Dev server bind address")

if(NOT EMSCRIPTEN AND NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    message(FATAL_ERROR
        "This project is now WebAssembly-only. Activate Emscripten and configure with:\n"
        "  emcmake cmake --preset wasm-debug\n"
        "or\n"
        "  emcmake cmake --preset wasm-release")
endif()

message(STATUS "Platform: Web (Emscripten/WASM)")
message(STATUS "Renderer: SDL3 + WebGL2")
message(STATUS "JavaScript Output: Modularized ES module")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

list(FILTER SOURCES EXCLUDE REGEX ".*/render_native\\.c$")
list(SORT SOURCES)

add_executable(testProject ${SOURCES})

target_include_directories(testProject PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/testProject
)

target_compile_options(testProject PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    "SHELL:-sUSE_SDL=3"
    "SHELL:-sUSE_WEBGL2=1"
    "SHELL:-sMIN_WEBGL_VERSION=2"
    "SHELL:-sMAX_WEBGL_VERSION=2"
)

target_link_options(testProject PRIVATE
    "SHELL:-sUSE_SDL=3"
    "SHELL:-sUSE_WEBGL2=1"
    "SHELL:-sMIN_WEBGL_VERSION=2"
    "SHELL:-sMAX_WEBGL_VERSION=2"
    "SHELL:-sALLOW_MEMORY_GROWTH=1"
    "SHELL:-sMODULARIZE=1"
    "SHELL:-sEXPORT_ES6=1"
    "SHELL:-sENVIRONMENT=web"
    "SHELL:-sEXPORTED_FUNCTIONS=['_main','_initWebGL','_startMainLoop','_myFunction']"
    "SHELL:-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
)

set_target_properties(testProject PROPERTIES
    SUFFIX ".js"
    OUTPUT_NAME "testProject"
)

if(BUILD_WASM_HTML)
    add_custom_command(TARGET testProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/html_template/index.html"
                "$<TARGET_FILE_DIR:testProject>/index.html"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/html_template/bootstrap.js"
                "$<TARGET_FILE_DIR:testProject>/bootstrap.js"
        VERBATIM
    )
endif()

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_custom_target(serve
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan
                "Serving ${CMAKE_CURRENT_BINARY_DIR} at http://${SERVE_HOST}:${SERVE_PORT}"
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}
                ${Python3_EXECUTABLE} -u -m http.server ${SERVE_PORT} --bind ${SERVE_HOST}
        DEPENDS testProject
        USES_TERMINAL
        VERBATIM
    )
endif()

message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Artifacts:")
message(STATUS "  - testProject.js")
message(STATUS "  - testProject.wasm")
if(BUILD_WASM_HTML)
    message(STATUS "  - index.html")
    message(STATUS "  - bootstrap.js")
endif()
message(STATUS "==================================")

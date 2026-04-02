cmake_minimum_required(VERSION 3.16)
project(tgv LANGUAGES C CXX)

file(GLOB CPP_SRCS src/*.cpp)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/functions)

set(IMGUI_SRCS
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
	external/imgui/backends/imgui_impl_opengl3.cpp
)

set(GL3W_SRC external/gl3w/gl3w.c)
set(GL3W_INCLUDE external/gl3w)

add_custom_target(copy_assets ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
)

add_executable(tgv
	${CPP_SRCS}
	${IMGUI_SRCS}
	${GL3W_SRC}
)

target_include_directories(tgv PRIVATE
	include
	external
	external/imgui
	external/imgui/backends
	${GL3W_INCLUDE}
)

find_package(OpenGL REQUIRED)
target_link_libraries(tgv PRIVATE
	glfw
	OpenGL::GL
	dl
)

target_compile_options(tgv PRIVATE -Wno-pragmas)
target_compile_definitions(tgv PRIVATE 
	PROJECT_INCLUDE_DIR="${CMAKE_SOURCE_DIR}/include"
	PROJECT_FUNCTIONS_DIR="${CMAKE_SOURCE_DIR}/build/functions"
)

if(UNIX AND NOT APPLE)
    set_target_properties(tgv PROPERTIES
        BUILD_RPATH "${CMAKE_SOURCE_DIR}/libs"
    )
endif()

add_dependencies(tgv copy_assets)

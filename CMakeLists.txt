cmake_minimum_required(VERSION 3.0)
project(gdal_viewer)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake/modules")

include(ImportProject)
include(HandmadeMath)
include(Sokol)
include(Stb)


set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


import_git_repository(stb https://github.com/nothings/stb.git)
import_stb(${stb_SOURCE_DIR} LIBRARIES image)

import_git_repository(hmm https://github.com/HandmadeMath/Handmade-Math.git)
import_handmademath(${hmm_SOURCE_DIR} ALIAS hmm)

import_git_repository(sokol https://github.com/floooh/sokol.git)
import_sokol(${sokol_SOURCE_DIR})

import_git_repository(sokol_tools https://github.com/floooh/sokol-tools-bin.git)
import_sokol_tools(${sokol_tools_SOURCE_DIR})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)
sokol_generate_shader(shaders/png_image.glsl ${CMAKE_BINARY_DIR}/shaders/png_image.h)

add_executable(imagevw
    main.c
    ${CMAKE_BINARY_DIR}/shaders/png_image.h
)
target_include_directories(imagevw PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(imagevw PRIVATE sokol hmm stb_image)
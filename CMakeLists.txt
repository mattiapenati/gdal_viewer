cmake_minimum_required(VERSION 3.0)
project(gdal_viewer)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake/modules")

include(ImportProject)
include(HandmadeMath)
include(Sokol)
include(ImGui)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


import_git_repository(hmm https://github.com/HandmadeMath/Handmade-Math.git)
import_handmademath(${hmm_SOURCE_DIR} ALIAS hmm)

import_git_repository(cimgui https://github.com/cimgui/cimgui.git)
import_cimgui(${cimgui_SOURCE_DIR})

import_git_repository(sokol https://github.com/floooh/sokol.git)
import_sokol(${sokol_SOURCE_DIR} LIBRARIES imgui IMGUI_TARGET cimgui)

import_git_repository(sokol_tools https://github.com/floooh/sokol-tools-bin.git)
import_sokol_tools(${sokol_tools_SOURCE_DIR})

find_package(GDAL REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)
sokol_generate_shader(shaders/rgba_shader.glsl ${CMAKE_BINARY_DIR}/shaders/rgba_shader.h)

add_executable(imagevw)
target_sources(imagevw PRIVATE
    raster.h
    raster.c
    base_raster.h
    rgba_raster.c
    gdal_raster.c
    ${CMAKE_BINARY_DIR}/shaders/rgba_shader.h
    main.c
)
target_include_directories(imagevw PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(imagevw PRIVATE sokol sokol_imgui hmm GDAL::GDAL)

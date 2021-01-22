include_guard()
set(SokolModule_SOURCES ${CMAKE_CURRENT_LIST_DIR})

include(CMakeParseArguments)

function(import_sokol SourceDir)
    set(options OPENGL)
    set(one_value_keywords LIBRARIES)
    set(multi_value_keywords IMGUI_TARGET)
    cmake_parse_arguments(ImportSokol
        "${options}" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN})

    set(ImportSokol_SOURCE_DIR "${SourceDir}")
    set(ImportSokol_BINARY_DIR "${CMAKE_BINARY_DIR}/_imported/sokol")

    define_property(TARGET
        PROPERTY SHADER_LANGUAGE
        BRIEF_DOCS "Shader language used by shdc"
        FULL_DOCS "Shader language used by shdc"
    )

    add_library(sokol STATIC EXCLUDE_FROM_ALL
        ${ImportSokol_SOURCE_DIR}/sokol_app.h
        ${ImportSokol_SOURCE_DIR}/sokol_gfx.h
        ${ImportSokol_SOURCE_DIR}/sokol_time.h
        ${ImportSokol_SOURCE_DIR}/sokol_audio.h
        ${ImportSokol_SOURCE_DIR}/sokol_fetch.h
        ${ImportSokol_SOURCE_DIR}/sokol_glue.h
    )
    target_include_directories(sokol PUBLIC ${ImportSokol_SOURCE_DIR})
    target_compile_definitions(sokol PRIVATE $<$<CONFIG:Debug>:SOKOL_TRACE_HOOKS>)
    set_target_properties(sokol PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${ImportSokol_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${ImportSokol_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${ImportSokol_BINARY_DIR}/bin"
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_sources(sokol PRIVATE ${SokolModule_SOURCES}/sokol.m)
        target_link_libraries(sokol PRIVATE "-framework Cocoa;-framework QuartzCore;-framework AudioToolbox")
        if(ImportSokol_OPENGL)
            target_compile_definitions(sokol PUBLIC SOKOL_GLCORE33)
            target_link_libraries(sokol PRIVATE "-framework OpenGL")
            set_target_properties(sokol PROPERTIES SHADER_LANGUAGE "glsl330")
        else()
            target_compile_definitions(sokol PUBLIC SOKOL_METAL)
            target_link_libraries(sokol PRIVATE "-framework Metal;-framework MetalKit")
            set_target_properties(sokol PROPERTIES SHADER_LANGUAGE "metal_macos")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_sources(sokol PRIVATE ${SokolModule_SOURCES}/sokol.c)
        target_compile_definitions(sokol PUBLIC SOKOL_D3D11 SOKOL_WIN32_FORCE_MAIN)
        set_target_properties(sokol PROPERTIES SHADER_LANGUAGE "hlsl4")
    else()
        message(FATAL_ERROR "Unsupported platform ${CMAKE_SYSTEM_NAME}")
    endif()

    foreach(ImportSokol_LIBRARY IN ITEMS ${ImportSokol_LIBRARIES})
        if(ImportSokol_LIBRARY STREQUAL imgui)
            if(NOT DEFINED ImportSokol_IMGUI_TARGET)
                if(NOT TARGET imgui)
                    message(FATAL_ERROR "imgui target is not defined")
                endif()
                set(ImportSokol_IMGUI_TARGET imgui)
            endif()

            add_library(sokol_imgui STATIC EXCLUDE_FROM_ALL
                ${ImportSokol_SOURCE_DIR}/util/sokol_imgui.h
            )
            target_link_libraries(sokol_imgui
                PUBLIC sokol ${ImportSokol_IMGUI_TARGET})
            target_include_directories(sokol_imgui
                PUBLIC ${ImportSokol_SOURCE_DIR}/util)
            set_target_properties(sokol_imgui PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY "${ImportSokol_BINARY_DIR}/lib"
                LIBRARY_OUTPUT_DIRECTORY "${ImportSokol_BINARY_DIR}/lib"
                RUNTIME_OUTPUT_DIRECTORY "${ImportSokol_BINARY_DIR}/bin"
            )

            if(ImportSokol_IMGUI_TARGET STREQUAL imgui)
                target_sources(sokol_imgui PRIVATE ${SokolModule_SOURCES}/sokol_imgui.cpp)
            elseif(ImportSokol_IMGUI_TARGET STREQUAL cimgui)
                target_sources(sokol_imgui PRIVATE ${SokolModule_SOURCES}/sokol_imgui.c)
            else()
                message(FATAL_ERROR "Unknown ImGui target ${ImportSokol_IMGUI_TARGET}")
            endif()
        else()
            message(FATAL_ERROR "Unsupported sokol library ${ImportSokol_LIBRARY}")
        endif()
    endforeach()
endfunction()

function(import_sokol_tools SourceDir)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(SokolTools_SHDC "${SourceDir}/bin/osx/sokol-shdc")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(SokolTools_SHDC "${SourceDir}/bin/linux/sokol-shdc")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(SokolTools_SHDC "${SourceDir}/bin/win32/sokol-shdc.exe")
    else()
        message(FATAL_ERROR "Unsupported platform ${CMAKE_SYSTEM_NAME}")
    endif()

    if(NOT EXISTS ${SokolTools_SHDC})
        message(FATAL_ERROR "Shader code generator not found")
    endif()

    set(SHDC_EXECUTABLE "${SokolTools_SHDC}"
        CACHE FILEPATH "Shader code generator" FORCE)
    mark_as_advanced(SHDC_EXECUTABLE)
endfunction()

function(sokol_generate_shader InputFile OutputFile)
    if(NOT TARGET sokol)
        message(FATAL_ERROR "Sokol has not been imported")
    endif()
    if(NOT DEFINED SHDC_EXECUTABLE)
        message(FATAL_ERROR "Shader code generator not found")
    endif()

    get_filename_component(SokolGenerateShader_INPUT_FILE ${InputFile} ABSOLUTE)
    get_target_property(SokolGenerateShader_SLANG sokol SHADER_LANGUAGE)

    set(SokolGenerateShader_ARGS)
    list(APPEND SokolGenerateShader_ARGS "--input=${SokolGenerateShader_INPUT_FILE}")
    list(APPEND SokolGenerateShader_ARGS "--output=${OutputFile}")
    list(APPEND SokolGenerateShader_ARGS "--format=sokol")
    list(APPEND SokolGenerateShader_ARGS "--bytecode")
    if(MSVC)
        list(APPEND SokolGenerateShader_ARGS "--errfmt=msvc")
    else()
        list(APPEND SokolGenerateShader_ARGS "--errfmt=gcc")
    endif()
    list(APPEND SokolGenerateShader_ARGS "--slang=${SokolGenerateShader_SLANG}")

    add_custom_command(
        OUTPUT ${OutputFile}
        COMMAND ${CMAKE_COMMAND} -E env ${SHDC_EXECUTABLE} ${SokolGenerateShader_ARGS}
        MAIN_DEPENDENCY ${SokolGenerateShader_INPUT_FILE}
        COMMENT "Compiling shader ${InputFile}"
        VERBATIM
    )
endfunction()

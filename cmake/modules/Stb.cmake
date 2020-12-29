include_guard()
set(StbModule_SOURCES ${CMAKE_CURRENT_LIST_DIR})

include(CMakeParseArguments)

function(import_stb SourceDir)
    set(options "")
    set(one_value_keywords "")
    set(multi_value_keywords LIBRARIES)
    cmake_parse_arguments(ImportStb
        "${options}" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN})

    set(ImportStb_SOURCE_DIR "${SourceDir}")
    set(ImportStb_BINARY_DIR "${CMAKE_BINARY_DIR}/_imported/stb")

    foreach(ImportStb_LIBRARY IN ITEMS ${ImportStb_LIBRARIES})
        add_library(stb_${ImportStb_LIBRARY} STATIC EXCLUDE_FROM_ALL)
        target_include_directories(stb_${ImportStb_LIBRARY} PUBLIC ${SourceDir})
        set_target_properties(stb_${ImportStb_LIBRARY} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${ImportStb_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${ImportStb_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${ImportStb_BINARY_DIR}/bin"
        )

        if(ImportStb_LIBRARY STREQUAL "image")
            target_sources(stb_image PRIVATE
                ${ImportStb_SOURCE_DIR}/stb_image.h
                ${StbModule_SOURCES}/stb_image.c
            )
        else()
            message(FATAL_ERROR "Unsupported stb library ${ImportStb_LIBRARY}")
        endif()
    endforeach()
endfunction(import_stb)

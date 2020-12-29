include_guard()
set(HandmadeMathModule_SOURCES ${CMAKE_CURRENT_LIST_DIR})

include(CMakeParseArguments)

function(import_handmademath SourceDir)
    set(options "")
    set(one_value_keywords ALIAS)
    set(multi_value_keywords "")
    cmake_parse_arguments(ImportHandmadeMath
        "${options}" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN})

    set(ImportHandmadeMath_SOURCE_DIR "${SourceDir}")
    set(ImportHandmadeMath_BINARY_DIR "${CMAKE_BINARY_DIR}/_imported/handmademath")

    add_library(handmademath STATIC EXCLUDE_FROM_ALL
        ${HandmadeMathModule_SOURCES}/hmm.c
        ${ImportHandmadeMath_SOURCE_DIR}/HandmadeMath.h
    )
    target_include_directories(handmademath PUBLIC ${SourceDir})
    set_target_properties(handmademath PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${ImportHandmadeMath_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${ImportHandmadeMath_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${ImportHandmadeMath_BINARY_DIR}/bin"
    )

    if(DEFINED ImportHandmadeMath_ALIAS)
        add_library(${ImportHandmadeMath_ALIAS} ALIAS handmademath)
    endif()
endfunction()

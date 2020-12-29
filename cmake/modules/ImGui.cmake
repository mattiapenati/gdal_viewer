include_guard()

function(import_cimgui SourceDir)
    set(ImportCImGui_SOURCE_DIR "${SourceDir}")
    set(ImportCImGui_BINARY_DIR "${CMAKE_BINARY_DIR}/_imported/cimgui")

    add_subdirectory(${ImportCImGui_SOURCE_DIR})
    target_compile_definitions(cimgui INTERFACE CIMGUI_DEFINE_ENUMS_AND_STRUCTS)
    set_target_properties(cimgui PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        ARCHIVE_OUTPUT_DIRECTORY "${ImportCImGui_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${ImportCImGui_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${ImportCImGui_BINARY_DIR}/bin"
    )
endfunction()

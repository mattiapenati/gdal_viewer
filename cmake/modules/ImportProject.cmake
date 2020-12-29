include_guard()
set(ImportProjectModule_SCRIPTS ${CMAKE_CURRENT_LIST_DIR})

find_package(Git REQUIRED QUIET)
include(CMakeParseArguments)

function(import_git_repository Name Repository)
    set(options "")
    set(one_value_keywords BRANCH)
    set(multi_value_keywords "")
    cmake_parse_arguments(ImportGitRepository
        "${options}" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN})

    set(ImportGitRepository_SOURCE_DIR ${CMAKE_SOURCE_DIR}/_imported/${Name})

    # cloning repository
    if(NOT EXISTS "${ImportGitRepository_SOURCE_DIR}")
        set(GitClone_ARGS)
        list(APPEND GitClone_ARGS --depth 1)
        if(DEFINED ImportGitRepository_BRANCH)
            list(APPEND GitClone_ARGS -b "${ImportGitRepository_BRANCH}")
        endif()
        list(APPEND GitClone_ARGS ${Repository} ${ImportGitRepository_SOURCE_DIR})

        message(STATUS "Cloning ${Repository}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E env ${GIT_EXECUTABLE} clone ${GitClone_ARGS}
            RESULT_VARIABLE GitClone_RESULT
            OUTPUT_VARIABLE GitClone_OUTPUT
            ERROR_VARIABLE GitClone_ERROR
        )
        if(NOT GitClone_RESULT EQUAL 0)
            message(FATAL_ERROR "Git clone error\n${GitClone_ERROR}")
        endif()
    else()
        if(NOT EXISTS "${ImportGitRepository_SOURCE_DIR}/.git")
            message(FATAL_ERROR
                "Directory ${ImportGitRepository_SOURCE_DIR} already exists, but it is not a valid git repository")
        endif()
    endif()

    # update target
    set(ImportGitRepository_UPDATE_TARGET "imported_${Name}_update")
    add_custom_target(${ImportGitRepository_UPDATE_TARGET}
        COMMAND ${CMAKE_COMMAND} -E env ${GIT_EXECUTABLE} pull origin
        WORKING_DIRECTORY ${ImportGitRepository_SOURCE_DIR}
        COMMEND "Updating ${Name} project"
        VERBATIM
    )
    if(NOT TARGET imported_update)
        add_custom_target(imported_update)
    endif()
    add_dependencies(imported_update ${ImportGitRepository_UPDATE_TARGET})

    # status target
    set(ImportGitRepository_STATUS_TARGET "imported_${Name}_status")
    add_custom_target(${ImportGitRepository_STATUS_TARGET}
        COMMAND ${CMAKE_COMMAND} -DImportGitRepository_NAME=${Name}
                                 -DImportGitRepository_SOURCE_DIR=${ImportGitRepository_SOURCE_DIR}
                                 -P ${ImportProjectModule_SCRIPTS}/ImportGitRepository_Status.cmake
        VERBATIM
    )
    if(NOT TARGET imported_status)
        add_custom_target(imported_status)
    endif()
    add_dependencies(imported_status ${ImportGitRepository_STATUS_TARGET})

    # forward variables to parent scope
    set(${Name}_SOURCE_DIR "${ImportGitRepository_SOURCE_DIR}" PARENT_SCOPE)
endfunction()

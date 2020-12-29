if(NOT DEFINED ImportGitRepository_NAME)
    message(FATAL_ERROR "Missing ImportGitRepository_NAME variable")
endif()
if(NOT DEFINED ImportGitRepository_SOURCE_DIR)
    message(FATAL_ERROR "Missing ImportGitRepository_SOURCE_DIR variable")
endif()
if(NOT EXISTS "${ImportGitRepository_SOURCE_DIR}/.git")
    message(FATAL_ERROR "ImportGitRepository_SOURCE_DIR is not a git repository")
endif()


find_package(Git REQUIRED QUIET)
macro(git)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env ${GIT_EXECUTABLE} ${ARGV}
        WORKING_DIRECTORY ${ImportGitRepository_SOURCE_DIR}
        RESULT_VARIABLE Git_RESULT
        OUTPUT_VARIABLE Git_OUTPUT
        ERROR_VARIABLE Git_ERROR
    )
    if(NOT Git_RESULT EQUAL 0)
        message(FATAL_ERROR "Git error\n${Git_ERROR}")
    endif()
endmacro()


git(rev-parse --abbrev-ref HEAD)
string(STRIP "${Git_OUTPUT}" ImportGitRepository_BRANCH)

git(rev-parse HEAD)
string(STRIP "${Git_OUTPUT}" ImportGitRepository_LOCAL_REV)

git(ls-remote --heads origin ${ImportGitRepository_BRANCH})
string(SUBSTRING "${Git_OUTPUT}" 0 40 ImportGitRepository_REMOTE_REV)


execute_process(
    COMMAND ${CMAKE_COMMAND} -E env CLICOLOR_FORCE=1
        ${CMAKE_COMMAND} -E cmake_echo_color --no-newline "-- ${ImportGitRepository_NAME}: ")
if(ImportGitRepository_LOCAL_REV STREQUAL ImportGitRepository_REMOTE_REV)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env CLICOLOR_FORCE=1
            ${CMAKE_COMMAND} -E cmake_echo_color --green "uptodate")
else()
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env CLICOLOR_FORCE=1
            ${CMAKE_COMMAND} -E cmake_echo_color --red "branches out of sync")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E echo "   ${ImportGitRepository_BRANCH}: ${ImportGitRepository_LOCAL_REV}")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E echo "   origin/${ImportGitRepository_BRANCH}: ${ImportGitRepository_REMOTE_REV}")
endif()

add_subdirectory (googletest)
include_directories (../include googletest/googletest/include)

file (GLOB SRC *)

add_executable (unit-test ${SRC})
target_link_libraries (unit-test alenka-file gtest gtest_main)

# Download test files if needed. This is a workaround for GitHub's screwed up LFS system.
add_custom_command(TARGET PRE_BUILD unit-test COMMAND bash download-data.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/unit-test/data)

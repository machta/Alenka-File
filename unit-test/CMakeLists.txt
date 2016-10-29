# Build Google Test library.
set(BUILD_GMOCK OFF)
set(BUILD_GTEST ON)
add_subdirectory(googletest)
include_directories(../include googletest/googletest/include)

# The tests.
file(GLOB SRC *)
add_executable (unit-test ${SRC})
target_link_libraries (unit-test ${LIBS_TO_LINK_ALENKA_FILE} gtest gtest_main)

# Download test files if needed. This is a workaround for GitHub's screwed up LFS system.
add_custom_command(TARGET PRE_BUILD unit-test COMMAND bash download-data.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/unit-test/data)

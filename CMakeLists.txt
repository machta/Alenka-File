cmake_minimum_required(VERSION 3.1)
project(Alenka-File)

# Options.
option(BUILD_TESTS_ALENKA_FILE "Builds unit tests for Alenka-File." OFF)
set(CMAKE_CXX_STANDARD 11)
set(BUILD_SHARED_LIBS off CACHE BOOL "")
include_directories(boost)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_definitions(-D_GLIBCXX_DEBUG)
	endif()
endif()

# First build all the library dependencies.
include_directories(EDFlib)

set(BUILD_SHARED_LIBS false)
# This macro turns on my changes in the library that handle a newer version of the GDF format.
add_definitions(-DALLOW_GDF_V_251)
add_subdirectory(libgdf)
include_directories(libgdf/libgdf/include)

add_subdirectory(pugixml)
include_directories(pugixml/src)

# If you want to use this library, you need to link to these libraries.
set(LIBS_TO_LINK_ALENKA_FILE alenka-file GDF pugixml)
set(LIBS_TO_LINK_ALENKA_FILE ${LIBS_TO_LINK_ALENKA_FILE} PARENT_SCOPE)

# The library.
file(GLOB SRC src/*)
file(GLOB SRC_BOOST_S boost/libs/system/src/*.cpp)
file(GLOB SRC_BOOST_FS boost/libs/filesystem/src/*.cpp)

add_library(alenka-file STATIC ${SRC} ${SRC_BOOST_S} ${SRC_BOOST_FS})

if(MSVC)
	set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS "-W4")
	add_definitions(-DBOOST_FILESYSTEM_NO_LIB -DBOOST_SYSTEM_NO_LIB) # This stops Microsoft's linker from needing the static libraries.
else()
	set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS "-Wall -pedantic -Wextra -Wconversion")
endif()

include_directories(include)

if(BUILD_TESTS_ALENKA_FILE)
	add_subdirectory(unit-test)
endif()

add_custom_target(install-alenka-file
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/lib
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:alenka-file> $<TARGET_FILE:GDF> $<TARGET_FILE:pugixml> ${CMAKE_INSTALL_PREFIX}/lib
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include ${CMAKE_INSTALL_PREFIX}/include)

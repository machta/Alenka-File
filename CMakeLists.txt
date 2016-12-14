cmake_minimum_required(VERSION 3.1)
project(Alenka-File)

# Options.
option(BUILD_TESTS_ALENKA_FILE "Builds unit tests for Alenka-File." OFF)
set(CMAKE_CXX_STANDARD 11)
set(BUILD_SHARED_LIBS false)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# First build all the library dependencies.
add_subdirectory(EDFlib)
include_directories(EDFlib)

set(BUILD_SHARED_LIBS false)
# This macro turns on my changes in the library that handle a newer version of the GDF format.
add_definitions(-DALLOW_GDF_V_251)
add_subdirectory(libgdf)
set_target_properties(gdf_merger PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
include_directories(libgdf/libgdf/include)

add_subdirectory(pugixml)
include_directories(pugixml/src)

# If you want to use this library, you need to link to these libraries.
set(LIBS_TO_LINK_ALENKA_FILE alenka-file EDF GDF pugixml)
set(LIBS_TO_LINK_ALENKA_FILE ${LIBS_TO_LINK_ALENKA_FILE} PARENT_SCOPE)

# The library.
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -pedantic")
endif()

include_directories(include ${BOOST_ROOT})

if(BUILD_TESTS_ALENKA_FILE)
	add_subdirectory (unit-test)
endif (BUILD_TESTS_ALENKA_FILE)

file(GLOB SRC src/*.cpp)
add_library(alenka-file ${SRC})

add_custom_target(install-alenka-file
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/lib
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:alenka-file> $<TARGET_FILE:EDF> $<TARGET_FILE:GDF> $<TARGET_FILE:pugixml> ${CMAKE_INSTALL_PREFIX}/lib
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include ${CMAKE_INSTALL_PREFIX}/include)

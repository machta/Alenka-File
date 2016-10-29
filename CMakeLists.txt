cmake_minimum_required(VERSION 3.1)
project(Alenka-File)

# Options.
option(BUILD_TESTS_ALENKA_FILE "Builds unit tests for Alenka-File." OFF)
set(CMAKE_CXX_STANDARD 11)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# First build all the library dependencies.
add_subdirectory(EDFlib)
set(INC ${INC} EDFlib)

set(BUILD_SHARED_LIBS false)
# This macro turns on my changes in the library that handle a newer version of the GDF format.
add_definitions(-DALLOW_GDF_V_251)
add_subdirectory(libgdf)
set_target_properties(gdf_merger PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
set(INC ${INC} libgdf/libgdf/include)

add_subdirectory(pugixml)
set(INC ${INC} pugixml/src)

# If you include this library into your project, you will want to link to these libraries.
set(LIBS_TO_LINK alenka-file EDF GDF pugixml)

# The library.
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -pedantic")
endif()

include_directories(include ${INC} ${BOOST_ROOT})

if(BUILD_TESTS_ALENKA_FILE)
	add_subdirectory (unit-test)
endif (BUILD_TESTS_ALENKA_FILE)

file(GLOB SRC src/*)
add_library(alenka-file ${SRC})

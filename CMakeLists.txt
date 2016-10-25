cmake_minimum_required(VERSION 3.0.0)
project(Alenka-File)

# Options.
option(BUILD_TEST "Builds unit tests." OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -pedantic")

# First build all the library dependencies.
add_subdirectory(EDFlib)
set(INC ${INC} EDFlib)

set(BUILD_SHARED_LIBS false)
add_subdirectory(libgdf)
set(INC ${INC} libgdf/libgdf/include)

# If you include this library into your project, you will want to link to these libraries.
set(LIBS_TO_LINK alenka-file EDF GDF)

# Tests.
if(BUILD_TEST)
	add_subdirectory (unit-test)
endif (BUILD_TEST)

# The library.
include_directories(include ${INC})
file(GLOB SRC src/*)
add_library(alenka-file ${SRC})

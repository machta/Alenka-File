cmake_minimum_required(VERSION 3.1)
project(Alenka-File)

# Options.
option(BUILD_TESTS_ALENKA_FILE "Builds unit tests for Alenka-File." OFF)
set(CMAKE_CXX_STANDARD 11)
set(BUILD_SHARED_LIBS off CACHE BOOL "")

# First build all the library dependencies.
include_directories(boost)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	include_directories(matio/src matio/visual_studio)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		link_directories(matio/matio-msvc2015/x64)
	else()
		link_directories(matio/matio-msvc2015/x86)
	endif()
endif()

include_directories(EDFlib)

set(BUILD_SHARED_LIBS false) # TODO: Check if this is needed.

add_subdirectory(pugixml)
include_directories(pugixml/src)

# If you want to use this library, you need to link to these libraries.
set(LIBS_TO_LINK_ALENKA_FILE alenka-file pugixml matio)
set(LIBS_TO_LINK_ALENKA_FILE ${LIBS_TO_LINK_ALENKA_FILE} PARENT_SCOPE)

# Alenka-File library.
file(GLOB SRC_BOOST_S boost/libs/system/src/*.cpp)
file(GLOB SRC_BOOST_FS boost/libs/filesystem/src/*.cpp)

set(SRC
	include/AlenkaFile/abstractdatamodel.h
	include/AlenkaFile/datafile.h
	include/AlenkaFile/datamodel.h
	include/AlenkaFile/edf.h
	include/AlenkaFile/gdf2.h
	include/AlenkaFile/mat.h
	src/datafile.cpp
	src/datamodel.cpp
	src/edf.cpp
	src/edflib_extended.cpp
	src/edflib_extended.h
	src/gdf2.cpp
	src/mat.cpp
)

add_library(alenka-file STATIC ${SRC} ${SRC_BOOST_S} ${SRC_BOOST_FS})

if(BUILD_TESTS_ALENKA_FILE)
	add_subdirectory(unit-test)
endif()

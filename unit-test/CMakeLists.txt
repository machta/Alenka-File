# Build Google Test library.
set(SRC_GTEST googletest/googletest/src/gtest-all.cc googletest/googletest/src/gtest_main.cc)
include_directories(googletest/googletest/include googletest/googletest)

# The tests.
file(GLOB SRC *.cpp *.h)
add_executable(unit-test ${SRC} ${SRC_GTEST})

find_package (Threads)
target_link_libraries(unit-test ${LIBS_TO_LINK_ALENKA_FILE} matio ${CMAKE_THREAD_LIBS_INIT})

if(MSVC)
	set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS "-W4")

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		add_custom_command(TARGET unit-test POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/matio/matio-msvc2015/x64/hdf5.dll"
			"${PROJECT_SOURCE_DIR}/matio/matio-msvc2015/x64/libmatio.dll"
			"${PROJECT_SOURCE_DIR}/matio/matio-msvc2015/x64/szip.dll"
			"${PROJECT_SOURCE_DIR}/matio/matio-msvc2015/x64/zlib.dll"
			$<TARGET_FILE_DIR:unit-test>)
	else()
		add_custom_command(TARGET unit-test POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/matio/matio-msvc2015/x86/hdf5.dll"
			"${PROJECT_SOURCE_DIR}/matio/matio-msvc2015/x86/libmatio.dll"
			"${PROJECT_SOURCE_DIR}/matio/matio-msvc2015/x86/szip.dll"
			"${PROJECT_SOURCE_DIR}/matio/matio-msvc2015/x86/zlib.dll"
			$<TARGET_FILE_DIR:unit-test>)
	endif()
else()
	set_source_files_properties(${SRC} PROPERTIES COMPILE_FLAGS "-Wall -pedantic -Wextra -Wconversion")
endif()

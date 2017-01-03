#!/bin/bash

#git clone --depth 1 https://github.com/mbillingr/libgdf.git
git clone --depth 1 https://github.com/machta/libgdf.git -b update-gdf-v2.51
sed -ri 's/(find_package.*)/#\1/' libgdf/libgdf/CMakeLists.txt
sed -ri 's/(add_subdirectory.*tools.*)/#\1/' libgdf/CMakeLists.txt

git clone --depth 1 https://github.com/Teuniz/EDFlib.git
echo 'add_definitions(-D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE)' > EDFlib/CMakeLists.txt
echo 'add_library(EDF edflib.c)' >> EDFlib/CMakeLists.txt

git clone --depth 1 https://github.com/zeux/pugixml.git

git clone --depth 1 https://github.com/google/googletest.git unit-test/googletest

git clone --depth 1 https://github.com/boostorg/boost.git
cd boost # TODO: figure out how to do a shallow clone of the submodules
git submodule update --init -- libs/program_options libs/config libs/any libs/type_index libs/static_assert libs/throw_exception libs/assert libs/core libs/type_traits libs/mpl libs/preprocessor libs/function libs/integer libs/bind libs/move libs/lexical_cast libs/range libs/iterator libs/detail libs/concept_check libs/utility libs/numeric/conversion libs/array libs/functional libs/container libs/math libs/predef libs/smart_ptr libs/tokenizer
cp -r libs/*/include/boost libs/*/*/include/boost .
cd -


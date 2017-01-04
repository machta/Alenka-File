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

BOOST=boost_1_63_0
curl -L https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.zip > $BOOST.zip
unzip -q $BOOST.zip
rm $BOOST.zip
mkdir -p boost/libs
mv $BOOST/boost boost
mv $BOOST/libs/program_options boost/libs
rm -rf $BOOST

# This version doesn't work in git-bash but is faster.
#BOOST=boost_1_63_0
#curl -L https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.zip > $BOOST.zip
#unzip -q $BOOST.zip $BOOST/boost/* $BOOST/libs/program_options/*
#rm $BOOST.zip
#mv $BOOST boost

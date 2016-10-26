#!/bin/bash

git clone https://github.com/mbillingr/libgdf.git

git clone https://github.com/Teuniz/EDFlib.git
echo 'add_definitions(-D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE)' > EDFlib/CMakeLists.txt
echo 'add_library(EDF edflib.c)' > EDFlib/CMakeLists.txt

git clone https://github.com/zeux/pugixml.git

git clone https://github.com/google/googletest.git unit-test/googletest

#!/bin/bash

# Usage: ./download-libraries.sh
#
# This script downloads all dependant libraries.
# Use Git Bash or a similar tool to run this on Windows.

if [ -d libgdf ]
then
	libgdf=skipped
else
	git clone --depth 1 https://github.com/mbillingr/libgdf.git &&
	sed -ri 's/(find_package.*)/#\1/' libgdf/libgdf/CMakeLists.txt &&
	sed -ri 's/(add_subdirectory.*tools.*)/#\1/' libgdf/CMakeLists.txt &&
	libgdf=OK || libgdf=fail
fi

if [ -d EDFlib ]
then
	EDFlib=skipped
else
	git clone --depth 1 https://github.com/Teuniz/EDFlib.git &&
	EDFlib=OK || EDFlib=fail
fi

if [ -d pugixml ]
then
	pugixml=skipped
else
	git clone --depth 1 https://github.com/zeux/pugixml.git &&
	pugixml=OK || pugixml=fail
fi

if [ -d unit-test/googletest ]
then
	googletest=skipped
else
	git clone --depth 1 https://github.com/google/googletest.git unit-test/googletest &&
	googletest=OK || googletest=fail
fi

if [ -d boost ]
then
	boost=skipped
else
	BOOST=boost_1_63_0 &&
	curl -L https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.zip > $BOOST.zip &&
	unzip -q $BOOST.zip &&
	rm $BOOST.zip &&
	mkdir -p boost/libs &&
	mv $BOOST/boost boost &&
	mv $BOOST/libs/program_options boost/libs &&
	mv $BOOST/libs/system boost/libs &&
	mv $BOOST/libs/filesystem boost/libs &&
	rm -rf $BOOST &&
	boost=OK || boost=fail

	# This version doesn't work in git-bash but is faster.
	#BOOST=boost_1_63_0 &&
	#curl -L https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.zip > $BOOST.zip &&
	#unzip -q $BOOST.zip $BOOST/boost/* $BOOST/libs/program_options/* &&
	#rm $BOOST.zip &&
	#mv $BOOST boost
fi

echo
echo ========== Download summary ==========
echo "Library path            Status"
echo ======================================
echo "libgdf                  $libgdf"
echo "EDFlib                  $EDFlib"
echo "pugixml                 $pugixml"
echo "unit-test/googletest    $googletest"
echo "boost                   $boost"


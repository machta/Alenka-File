#!/bin/bash

# Usage: ./download-data.sh
#
# This script downloads the test files from Dropbox.
# This is a workaround for GitHub's screwed up LFS system.
# Use Git Bash or a similar tool to run this on Windows.

function download
{
	curl -L 'https://www.dropbox.com/s/'$1/$2'.zip?dl=0' > $2.zip &&
	unzip -o $2.zip &&
	rm $2.zip
}

data0=skipped
data1=skipped
data2=skipped
data3=skipped

md5sum -c --quiet data.md5 ||
{
	{ download bf5eo0ljy4x5cng alenka-file-data0 && data0=OK || data0=fail; } &&
	{ download f60urex3dh3lu9w alenka-file-data1 && data1=OK || data1=fail; } &&
	{ download re70d67juu0bb4d alenka-file-data2 && data2=OK || data2=fail; } &&
	mv gdf/t00.gdf gdf/gdf00.gdf &&
	mv gdf/t00_values.dat gdf/gdf00_values.dat &&
	mv gdf/t01.gdf gdf/gdf01.gdf &&
	mv gdf/t01_values.dat gdf/gdf01_values.dat &&
	{ download 24hxbhi3y1nxh91 alenka-file-data3 && data3=OK || data3=fail; }
} &&
md5sum -c --quiet data.md5 &&
md5=OK || md5=fail

echo
echo ======= Download summary =======
echo "File                Status"
echo ================================
echo "data0               $data0"
echo "data1               $data1"
echo "data2               $data2"
echo "data3               $data3"
echo "md5sum              $md5"


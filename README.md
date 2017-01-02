# Alenka-File

### Requirments
* git
* cmake (for Ubuntu 14 download the latest version)

On debian-like systems you can use: `sudo apt install git cmake-gui build-essential`

### Build instructions
1. Clone this repo
2. Install libraries using download-libraries.sh
3. Make new build directory and change into it
4. Use cmake-gui to generate build environment
  1. Click Configure and choose a compiler
  2. Change CMAKE_BUILD_TYPE (not needed on Windows)
  3. Change CMAKE_INSTALL_PREFIX
  4. For a 32-bit build make sure BUILD64 is unset
  5. Click Generate
5. Build the library

Here is an example of the setup using git-bash (or regular bash):
``` bash
./download-libraries.sh
mkdir build-Release-64
cd build-Release-64
cmake .. -D "CMAKE_BUILD_TYPE=Release CMAKE_INSTALL_PREFIX=install-dir"
cmake --build . --config Release --target install-alenka-file
```

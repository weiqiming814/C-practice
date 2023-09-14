# WUZI Game

The first one to connect five chess pieces together on a 19*19 board wins.

## Requirements

This project is only tested on Linux, although this may work on OSx or WSL. It requires a working version of GCC, GDB and make in your path.

## Usage
### Get WUZI Game
Clone the PC project and the related sub modules:
```
git clone --recursive weiqiming/wuzi at master · weiqiming814/weiqiming (github.com)
```
## CMake
The following steps can be used with CMake on a Unix-like system. This may also work on other OSes but has not been tested.

1. Ensure CMake is installed, i.e. the `cmake` command works on the terminal.
2. Make a new directory. The name doesn't matter but `build` will be used for this tutorial.
3. Type `cd build`.
4. Type `cmake ..`. CMake will generate the appropriate build files.
5. Type `make -j4` or (more portable) `cmake --build . --parallel`.
6. The binary will be in `../build/wuzi`, and can be run by typing that command.

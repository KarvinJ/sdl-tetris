# pc-sdl-starter
A Windows SDL2 starter template. 

# Requirements
* Install the latest version of MinGW-w64 via MSYS2: [MinGW](https://code.visualstudio.com/docs/cpp/config-mingw) 
* MinGW-w64 version use: gcc-g++ versions (Rev3, Built by MSYS2 project) 13.2.0

# Building

## Debug Mode
To build this project in debug with all the errors showing, enter these commands:
```
cd bin/debug
make
```
or
```
cd bin/debug &&
mingw32-make
```
to build the project in ```debug``` mode.

## Release Mode
Sadly, the faster means more dangerous, since ```release``` mode doesn't have any ```-Wall``` flags, your project can exceed your memory and shut down your computer. Make sure to test in ```debug``` mode first.

To build this project:
```
cd bin/release
make
```
or
```
cd bin/release
mingw32-make
```
to build the project in the fastest mode to have optimizations.


# Credits
Thanks to [PrecisionChess](https://github.com/PrecisionChess/C-SDL2-Setup?tab=readme-ov-file) for the initial code.
# Touhou danmakufu ph3 (bgfx port)
Random attemps at porting this game into the world of crossplatform software.

***NOTE:*** Nothing works at this moment, nothing is usable at the moment with the exception of some engine tests (TestMode), look at the code at your own risk.

Steps for converting existing TouhouDanmakufu code for the new bgfx code could be viewed at [newbase documentation](https://github.com/arves100/danmakufu/tree/newbase/newbase).

Tested builds:
- Windows with MSVC 19.28 (32-bit)

No artifacts at this time, there might never be.

## Building

General requirements
- Cmake
- A C++ compiler that supports C++11 or greater (Gcc, Visual Studio, Clang, ...)

## Windows specific steps
1. Download and bootstrap [vcpkg](https://github.com/microsoft/vcpkg)
2. Install required vcpkg dependencies
`vcpkg install sdl2 zlib libvorbis --triplet=x86-windows-static` (or `vcpkg install sdl2 zlib libvorbis --triplet=x64-windows-static`)

3. Download the three repositores of bgfx
```
git clone https://github.com/bkaradzic/bgfx
git clone https://github.com/bkaradzic/bimg
git clone https://github.com/bkaradzic/bx
```

4. Open a command prompt, go to bgfx folder and type this command
`..\bx\tools\bin\windows\genie --with-tools --with-avx --with-windows=10 --with-platform=x32 vs2019` (or `--with-platform=x64`)

5. Open .build\projects\vs2019\bgfx.sln and make sure to build both Release and Debug of the following projects:
- shaderc
- bgfx
- bimg
- bx
- bimg_decode
 
## Linux specific steps
1. Install sdl2, libvorbis and zlib from your package manager
2. Download the three repositores of bgfx
```
git clone https://github.com/bkaradzic/bgfx
git clone https://github.com/bkaradzic/bimg
git clone https://github.com/bkaradzic/bx
```
3. Open a command prompt, go to bgfx folder and type this command
`../bx/tools/bin/linux/genie --with-tools --with-avx --with-platform=x32 gmake` (or `--with-platform=x64`)

NOTE: you can add `--with-wayland` if you want to enable wayland support, make sure to use an SDL2 build that supports wayland as well.

4. Go to .build/projects/gmake and build it with `make`, make sure to at least build the following projects:
- shaderc
- bgfx
- bimg
- bx
- bimg_decode

## Building the repo
Use CMake to generate the project, make sure to define the following variables: (either in cmake gui)
| Name | Type | Description | Example |
| ---- | ---- | ----------- | ------- |
| BGFX_INCLUDE_DIR | PATH |  Folder where the include directory of bgfx is placed | C:\bgfx\include |
| BGFX_LIBRARY_DEBUG | FILEPATH | Debug flavor of bgfx statc library | C:\bgfx\.build\win32_vs2019\bin\bgfxDebug.lib |
| BGFX_LIBRARY_RELEASE | FILEPATH | Release flavor of bgfx static library | C:\bgfx\.build\win32_vs2019\bin\bgfxRelease.lib |
| BIMG_INCLUDE_DIR | PATH | Folder where the include directory of bimg is placed | C:\bimg\include |
| BIMG_LIBRARY_DEBUG | FILEPATH | Debug flavor of bimg static library | C:\bgfx\.build\win32_vs2019\bin\bimgDebug.lib |
| BIMG_LIBRARY_RELEASE | FILEPATH | Release flavor of bimg static library | C:\bgfx\.build\win32_vs2019\bin\bimgRelease.lib |
| BX_INCLUDE_DIR | PATH | Folder where the include directory of bx is placed | C:\bx\include |
| BX_LIBRARY_DEBUG | FILEPATH | Debug flavor of bx statc library | C:\bgfx\.build\win32_vs2019\bin\bxDebug.lib |
| BX_LIBRARY_RELEASE | FILEPATH | Release flavor of bx static library | C:\bgfx\.build\win32_vs2019\bin\bxRelease.lib |
| BIMG_DECODE_LIBRARY_DEBUG | FILEPATH | Debug flavor of bimg_decode statc library | C:\bgfx\.build\win32_vs2019\bin\bimg_decodeDebug.lib |
| BIMG_DECODE_LIBRARY_RELEASE | FILEPATH | Release flavor of bimg_decode static library | C:\bgfx\.build\win32_vs2019\bin\bimg_decodeRelease.lib |
| BGFX_SHADERC | FILEPATH | path where the bgfx shader compiler executable exists | C:\bgfx\.build\win32_vs2019\bin\shadercRelease.exe |
| VCPKG_TARGET_TRIPLET | STRING | WINDOWS ONLY: Forces to use static MSVC | Either `x86-windows-static` or `x64-windows-static` |

## Credits
This project is a fork of [WishMakers0's Danmakufu Woo](https://github.com/WishMakers0/Danmakufu-Woo-Edition),
which is a modified version of the open source [mkm's Danmakufu ph3 [.1 pre6] ](https://touhougc.web.fc2.com/products/th_dnh_ph3.html).

The old repositories (both Woo edition and the original one) are released under NYSL Version 0.9982:
- [Japanese text of NYSL](http://www.kmonos.net/nysl/)
- [English text of NYSL](http://www.kmonos.net/nysl/index.en.html)

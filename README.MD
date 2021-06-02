# Danmakufu
Random experiments with this old game.

For notes about newbase, see newbase/README.MD

## Building
1. Install a C++ compiler:
   
Ubuntu: `sudo apt install g++-mingw-w64-i686`

Void Linux: `xbps-install -S cross-i686-w64-mingw32`

Windows: [Visual Studio](`https://visualstudio.microsoft.com/`)

2. Install CMake
3. Download and bootstrap [vcpkg](https://github.com/microsoft/vcpkg)
4. Install required dependences:

VisualStudio/MSVC:
`vcpkg install sdl2 zlib libvorbis --triplet=x86-windows-static`

Other platforms/MinGW:
```
cd (vcpkg directory)
git apply "(danmakufu directory)/_libvorbis_mingw.patch"
vcpkg install sdl2 zlib libvorbis --triplet=x86-mingw-dynamic
```

*NOTES:* mpg123 does not work with mingw, until libvorbis merges the patch we have to manually patch vcpkg (or send it to vcpkg to merge it)

5. Configure the project with vcpkg toolchain:
   
VisualStudio/MSVC:
`cmake -DCMAKE_TOOLCHAIN_FILE="(vcpkg directory/scripts/buildsystems/vcpkg.cmake)" "(danmakufu source)"`

Other platforms/MinGW:
`cmake -DVCPKG_TOOLCHAIN_FILE="(vcpkg directory/scripts/buildsystems/vcpkg.cmake)" -DCMAKE_TOOLCHAIN_FILE="(danmakufu source)/cmake/mingw.toolchain" "(danmakufu source)"`

6. Open the SLN on visual studio and build it or build it with make/ninja

## Testing
While I currently test with 32-bit builds on Windows, feel free to leave issues about 64-bit or Wine
support itself.



## Credits
This project is a fork of [WishMakers0's Danmakufu Woo](https://github.com/WishMakers0/Danmakufu-Woo-Edition),
which is a modified version of the open source [danmakufu ph3](https://touhougc.web.fc2.com/products/th_dnh_ph3.html).

The old repositories (both Woo edition and the original one) are released under NYSL Version 0.9982:
- [Japanese text of NYSL](http://www.kmonos.net/nysl/)
- [English text of NYSL](http://www.kmonos.net/nysl/index.en.html)

A new license for this changes is currently not selected.

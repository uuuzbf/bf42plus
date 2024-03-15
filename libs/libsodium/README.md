libsodium is built from source because the pre-built libraries are built with `Multi-threaded (/MT)` and not with `Multi-threaded DLL (/MD)` option.

To build libsodium:
- Download and unzip https://github.com/jedisct1/libsodium/archive/stable.zip
- Open builds\msvc\vs2022\libsodium.sln
- Select `StaticRelease` and `Win32` target
- In libsodium project properties go to `Configuration Properties -> C/C++ -> Code Generation -> Runtime Library` and set it to `Multi-threaded DLL (/MD)`
- Build the project
- From bin\Win32\Release\v143\static\ copy `libsodium.lib` and `libsodium.pdb` to this directory
- The include directory can be copied from a release at https://download.libsodium.org/libsodium/releases/ (find `*version*-stable-msvc.zip`)

This is a WS card game implementation with automated execution of card abilities.

## State of the project
Currently only Kaguya set is supported. You can download the latest release [here](https://github.com/longagofaraway/WSAmateur/releases). Automatic updates are supported on Windows. Visit discord for more info https://discord.gg/6hRd2VZ3vG

## Compiling

### Windows

Requirements: 
- Visual Studio 2019 at least 16.8
- Qt 5.15.2 (with support for msvc2019_64)
- vcpkg  
  - Using vcpkg install protobuf and openssl

#### Qt Creator

This is the recommended IDE, because it's easier to launch the program and to debug QML part of the code. Just open CMakeLists.txt as a project, set 'Initial CMake parameters' in 'Projects' tab and add 
```
-DCMAKE_TOOLCHAIN_FILE=F:\Projects\libs\vcpkg\scripts\buildsystems\vcpkg.cmake
-DCMAKE_PREFIX_PATH=F:\Soft\Qt\5.15.2\msvc2019_64\lib\cmake;F:\Projects\libs\vcpkg\installed\x64-windows\bin
```
with your local paths. Clear CMake cache and rerun CMake from QtCreator.

#### Visual Studio

Example cmake command:
```
cmake .. -DCMAKE_TOOLCHAIN_FILE=F:\Projects\libs\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_PREFIX_PATH="F:\Soft\Qt\5.15.2\msvc2019_64\lib\cmake;F:\Projects\libs\vcpkg\installed\x64-windows\bin" -G "Visual Studio 15 2019" -T "v142" -A x64
```

Build WSAmateur project. Then use windeployqt on the directory with compiled program. Example:
```
<QTDIR>\bin\qtenv2.bat
<QTDIR>\bin\windeployqt.exe --debug --qmldir <WSAmateurDir>\client\qml <WSAmateurDir>\build\client\Debug
```

Now the program should be ready to run.

### Linux

You can take some hints from .github/workflows/ci-linux-build.yml

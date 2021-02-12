This is a WS card game implementation with automated execution of card abilities.

Graphical part is QML and server implementation uses C++ coroutines.

## Install

Install Qt 5.15.2 and msvc that supports c++ coroutines (support added around september 2020)

Install vcpkg
Install protobuf using vcpkg

Example cmake command:
cmake .. -DCMAKE_TOOLCHAIN_FILE=F:\Projects\libs\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_PREFIX_PATH="F:\Soft\Qt\5.15.2\msvc2019_64\lib\cmake;F:\Projects\libs\vcpkg\installed\x64-windows\bin" -G "Visual Studio 15 2019" -T "v142" -A x64

Build WSAmateur project. Then use windeployqt on the directory with compiled program. Example:
<QTDIR>\bin\qtenv2.bat
<QTDIR>\bin\windeployqt.exe --debug --qmldir <WSAmateurDir>\client\qml <WSAmateurDir>\build\client\Debug

Now the program should be ready to run.
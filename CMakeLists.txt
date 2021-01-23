cmake_minimum_required(VERSION 3.5)

project(WSAmateur LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#enable C4996 after https://github.com/protocolbuffers/protobuf/issues/7243
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4251")
endif()

find_package(Qt5 COMPONENTS Core REQUIRED)

find_package(Protobuf REQUIRED)
if(NOT EXISTS "${Protobuf_PROTOC_EXECUTABLE}")
    message(FATAL_ERROR "No protoc command found!")
endif()

add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(abilities)

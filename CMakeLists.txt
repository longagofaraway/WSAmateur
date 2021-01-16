cmake_minimum_required(VERSION 3.5)

project(WSAmateur LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

find_package(Protobuf REQUIRED)
if(NOT EXISTS "${Protobuf_PROTOC_EXECUTABLE}")
    message(FATAL_ERROR "No protoc command found!")
endif()

add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(abilities)

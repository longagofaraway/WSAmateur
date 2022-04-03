cmake_minimum_required(VERSION 3.12)

project(WSAmateur VERSION 0.1.1)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#enable C4996 after https://github.com/protocolbuffers/protobuf/issues/7243
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4251")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(create-version-file)

find_package(Qt5 COMPONENTS Core REQUIRED)

find_package(Protobuf REQUIRED)
if(NOT EXISTS "${Protobuf_PROTOC_EXECUTABLE}")
    message(FATAL_ERROR "No protoc command found!")
endif()

if(WIN32)
    find_package(OpenSSL)
    if(NOT ${OPENSSL_FOUND})
        message(FATAL_ERROR "No openssl found!")
    endif()

    if(WIN32)
        find_package(Win32SslRuntime)
    endif()
endif()

if(WIN32)
    set(CPACK_GENERATOR NSIS ${CPACK_GENERATOR})
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)

    # Configure file with custom definitions for NSIS.
    configure_file(
        ${CMAKE_MODULE_PATH}/NSIS.definitions.nsh.in
        ${PROJECT_BINARY_DIR}/NSIS.definitions.nsh
    )
endif()

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${CMAKE_PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${CMAKE_PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${CMAKE_PROJECT_VERSION_PATCH}")
include (CPack)

add_subdirectory(client)
add_subdirectory(core)
add_subdirectory(abilities)
add_subdirectory(server)

install(FILES ${PROJECT_SOURCE_DIR}/cmake/update.cmd DESTINATION ./)

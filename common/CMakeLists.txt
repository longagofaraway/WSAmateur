add_subdirectory(proto)

set(commonSources
    server.cpp
    serverGame.cpp
    serverPlayer.cpp
    serverProtocolHandler.cpp
    connection.h
    server.h
    serverGame.h
    serverPlayer.h
    serverProtocolHandler.h
    )

add_library(wsamateurCommon STATIC ${commonSources})

target_include_directories(wsamateurCommon
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${Qt5Core_INCLUDE_DIRS}
)

target_compile_features(wsamateurCommon PRIVATE cxx_std_17)

target_link_libraries(wsamateurCommon PUBLIC wsamateurProto)


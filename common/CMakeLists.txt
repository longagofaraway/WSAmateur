add_subdirectory(proto)

set(commonSources
    cardDatabase.cpp
    cardInfo.cpp
    deckList.cpp
    server.cpp
    serverCard.cpp
    serverCardZone.cpp
    serverGame.cpp
    serverPlayer.cpp
    serverProtocolHandler.cpp
    cardDatabase.h
    cardInfo.h
    commands.h
    connection.h
    deckList.h
    server.h
    serverCard.h
    serverCardZone.h
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

if (MSVC)
    target_compile_options(wsamateurCommon PRIVATE "/wd4251")
endif()

target_link_libraries(wsamateurCommon PUBLIC wsamateurProto)


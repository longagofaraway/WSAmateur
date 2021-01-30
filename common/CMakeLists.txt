add_subdirectory(proto)

set(commonSources
    cardDatabase.cpp
    cardInfo.cpp
    deckList.cpp
    playAbility.cpp
    server.cpp
    serverCard.cpp
    serverCardZone.cpp
    serverGame.cpp
    serverPlayer.cpp
    serverProtocolHandler.cpp
    serverStage.cpp
    attributeChange.h
    cardDatabase.h
    cardInfo.h
    commands.h
    connection.h
    coroutineTask.h
    deckList.h
    server.h
    serverCard.h
    serverCardZone.h
    serverGame.h
    serverPlayer.h
    serverProtocolHandler.h
    serverStage.h
    globalAbilities/globalAbilities.h
    globalAbilities/globalAbilities.cpp
    )

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_library(wsamateurCommon STATIC ${commonSources})

target_include_directories(wsamateurCommon
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${QtCore_INCLUDE_DIRS}
)

target_compile_features(wsamateurCommon PRIVATE cxx_std_20)

target_link_libraries(wsamateurCommon PUBLIC wsamateurProto wsamateurAbilities
    PRIVATE Qt5::Core)


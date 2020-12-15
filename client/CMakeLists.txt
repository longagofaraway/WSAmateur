set(clientSources
    src/card.cpp
    src/cardModel.cpp
    src/cardZone.cpp
    src/client.cpp
    src/clientConnection.cpp
    src/commonCardZone.cpp
    src/deck.cpp
    src/game.cpp
    src/hand.cpp
    src/imageProvider.cpp
    src/localClientConnection.cpp
    src/localServer.cpp
    src/localServerConnection.cpp
    src/main.cpp
    src/player.cpp
    src/waitingRoom.cpp
    src/card.h
    src/cardModel.h
    src/cardZone.h
    src/client.h
    src/clientConnection.h
    src/commonCardZone.h
    src/deck.h
    src/game.h
    src/hand.h
    src/imageProvider.h
    src/localClientConnection.h
    src/localServer.h
    src/localServerConnection.h
    src/player.h
    src/waitingRoom.h)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(WSAmateur ${clientSources} qml.qrc)

if (MSVC)
    target_compile_options(WSAmateur PRIVATE "/wd4251")
endif()

target_compile_features(WSAmateur PRIVATE cxx_std_17)
target_compile_definitions(WSAmateur
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(WSAmateur PRIVATE wsamateurCommon
  PRIVATE Qt5::Core Qt5::Quick)

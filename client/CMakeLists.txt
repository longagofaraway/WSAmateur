set(clientSources
    src/card.cpp
    src/client.cpp
    src/clientConnection.cpp
    src/game.cpp
    src/hand.cpp
    src/localClientConnection.cpp
    src/localServer.cpp
    src/localServerConnection.cpp
    src/main.cpp
    src/player.cpp
    src/card.h
    src/client.h
    src/clientConnection.h
    src/game.h
    src/hand.h
    src/localClientConnection.h
    src/localServer.h
    src/localServerConnection.h
    src/player.h)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(WSAmateur ${clientSources} qml.qrc)
target_compile_features(WSAmateur PRIVATE cxx_std_17)
target_compile_definitions(WSAmateur
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(WSAmateur PRIVATE wsamateurCommon
  PRIVATE Qt5::Core Qt5::Quick)

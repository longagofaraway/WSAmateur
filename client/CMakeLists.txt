set(clientSources
    src/card.cpp
    src/client.cpp
    src/game.cpp
    src/hand.cpp
    src/localConnection.cpp
    src/localServer.cpp
    src/main.cpp
    src/player.cpp
    src/remoteConnection.cpp
    src/card.h
    src/client.h
    src/game.h
    src/hand.h
    src/localConnection.h
    src/localServer.h
    src/player.h
    src/remoteConnection.h)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Quick REQUIRED)

include_directories(../common)

add_executable(WSAmateur ${clientSources} qml.qrc)
target_compile_features(WSAmateur PRIVATE cxx_std_17)
target_compile_definitions(WSAmateur
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(WSAmateur PRIVATE wsamateurCommon
  PRIVATE Qt5::Core Qt5::Quick)

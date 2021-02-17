set(clientSources
    src/activatedAbilities.cpp
    src/activatedAbilityModel.cpp
    src/card.cpp
    src/cardModel.cpp
    src/cardZone.cpp
    src/choiceDialog.cpp
    src/client.cpp
    src/clientConnection.cpp
    src/commonCardZone.cpp
    src/deckView.cpp
    src/game.cpp
    src/hand.cpp
    src/imageProvider.cpp
    src/localClientConnection.cpp
    src/localServer.cpp
    src/localServerConnection.cpp
    src/main.cpp
    src/playAbility.cpp
    src/player.cpp
    src/stage.cpp
    src/abilityModel.cpp
    src/activatedAbilities.h
    src/activatedAbilityModel.h
    src/card.h
    src/cardModel.h
    src/cardZone.h
    src/choiceDialog.h
    src/client.h
    src/clientConnection.h
    src/commonCardZone.h
    src/deckView.h
    src/game.h
    src/hand.h
    src/imageProvider.h
    src/localClientConnection.h
    src/localServer.h
    src/localServerConnection.h
    src/player.h
    src/stage.h
    src/abilityModel.h)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(WSAmateur ${clientSources} qml.qrc)

target_compile_features(WSAmateur PRIVATE cxx_std_20)
target_compile_definitions(WSAmateur
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(WSAmateur PRIVATE wsamateurCommon
    PRIVATE Qt5::Core Qt5::Quick)

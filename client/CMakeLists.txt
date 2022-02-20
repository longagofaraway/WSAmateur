set(clientSources
    src/abilityEventProcessing.cpp
    src/abilityModel.cpp
    src/activatedAbilities.cpp
    src/activatedAbilityModel.cpp
    src/application.cpp
    src/card.cpp
    src/cardModel.cpp
    src/cardZone.cpp
    src/checkTiming.cpp
    src/choiceDialog.cpp
    src/client.cpp
    src/clientConnection.cpp
    src/commonCardZone.cpp
    src/deckMenuModel.cpp
    src/deckView.cpp
    src/decksListWindow.cpp
    src/deckUtils.cpp
    src/filesystemPaths.cpp
    src/game.cpp
    src/gamePreparation.cpp
    src/hand.cpp
    src/imageLinks.cpp
    src/imageLoader.cpp
    src/imageProvider.cpp
    src/lobby.cpp
    src/localClientConnection.cpp
    src/localConnectionManager.cpp
    src/localServerConnection.cpp
    src/main.cpp
    src/playAbility.cpp
    src/player.cpp
    src/publicServers.cpp
    src/remoteClientConnection.cpp
    src/settingsManager.cpp
    src/stage.cpp
    src/updater.cpp
    src/userListModel.cpp
    src/utils.cpp
    src/versionParser.cpp
    src/abilityModel.h
    src/activatedAbilities.h
    src/activatedAbilityModel.h
    src/application.h
    src/card.h
    src/cardModel.h
    src/cardZone.h
    src/choiceDialog.h
    src/client.h
    src/clientConnection.h
    src/commonCardZone.h
    src/deckMenuModel.h
    src/deckView.h
    src/decksListWindow.h
    src/filesystemPaths.h
    src/game.h
    src/gamePreparation.h
    src/deckUtils.h
    src/hand.h
    src/imageLinks.h
    src/imageLoader.h
    src/imageProvider.h
    src/lobby.h
    src/localClientConnection.h
    src/localConnectionManager.h
    src/localServerConnection.h
    src/player.h
    src/publicServers.h
    src/remoteClientConnection.h
    src/settingsManager.h
    src/stage.h
    src/updater.h
    src/userListModel.h
    src/utils.h
    src/versionParser.h
    ${VERSION_STRING_CPP})

if (WIN32)
    set(clientSources ${clientSources} wsamateur.rc)
endif()

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(WSAmateur WIN32 ${clientSources} qml.qrc)

add_definitions(-DTEST_MODE)

target_compile_features(WSAmateur PRIVATE cxx_std_20)
target_compile_definitions(WSAmateur
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(WSAmateur PRIVATE wsamateurCore
    PRIVATE Qt5::Core Qt5::Quick ${Protobuf_LIBRARIES})

install(TARGETS WSAmateur RUNTIME DESTINATION ./)

# Ensure the Qt runtime libraries are installed
if(WIN32)
    include(Windeployqt)
    windeployqt(WSAmateur ./)
    install(FILES "$<TARGET_FILE_DIR:WSAmateur>/libprotobuf.dll" DESTINATION ./)

    if(WIN32SSLRUNTIME_FOUND)
        install(FILES ${WIN32SSLRUNTIME_LIBRARIES} DESTINATION ./)
    endif()

endif()

add_subdirectory(proto)

set(coreSources
    abilityPlayer.cpp
    abilityUtils.cpp
    buff.cpp
    cardBuffs.cpp
    cardBuffManager.cpp
    cardDatabase.cpp
    cardInfo.cpp
    condition.cpp
    connectionManager.cpp
    deckList.cpp
    imageLinks.cpp
    lookRevealEffects.cpp
    moveCardEffect.cpp
    multipliers.cpp
    playAbility.cpp
    playEffect.cpp
    playHardcodedEffect.cpp
    playerBuffManager.cpp
    server.cpp
    serverCard.cpp
    serverCardZone.cpp
    serverGame.cpp
    serverPlayer.cpp
    serverProtocolHandler.cpp
    serverStage.cpp
    serverUser.cpp
    trigger.cpp
    triggerManager.cpp
    abilityContext.h
    abilityPlayer.h
    abilityUtils.h
    attributeChange.h
    cardBuffManager.h
    cardDatabase.h
    cardInfo.h
    cardImprint.h
    commands.h
    connection.h
    connectionManager.h
    coroutineTask.h
    deckList.h
    imageLinks.h
    playerBuffManager.h
    networkUtils.h
    server.h
    serverCard.h
    serverCardZone.h
    serverGame.h
    serverPlayer.h
    serverProtocolHandler.h
    serverStage.h
    serverUser.h
    triggerManager.h
    globalAbilities/delayedAbility.h
    globalAbilities/globalAbilities.h
    globalAbilities/globalAbilities.cpp
    hardcodedEffects/S79/20.cpp
    ${VERSION_STRING_CPP}
    )

find_package(Qt5 COMPONENTS Quick Sql REQUIRED)

add_library(wsamateurCore STATIC ${coreSources})

target_include_directories(wsamateurCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${QtCore_INCLUDE_DIRS}
)

target_compile_features(wsamateurCore PRIVATE cxx_std_20)
if (UNIX)
	target_compile_options(wsamateurCore PUBLIC -fcoroutines)
endif (UNIX)

target_link_libraries(wsamateurCore PUBLIC wsamateurProto wsamateurAbilities Qt5::Sql
    PRIVATE Qt5::Core)


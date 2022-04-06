set(abilityMakerSources
    abilityComponent.h
    abilityComponent.cpp
    abilityMaker.h
    abilityMaker.cpp
    arrayOfAbilitiesComponent.h
    arrayOfAbilitiesComponent.cpp
    arrayOfConditionsComponent.h
    arrayOfConditionsComponent.cpp
    arrayOfEffectsComponent.h
    arrayOfEffectsComponent.cpp
    baseComponent.h
    baseComponent.cpp
    cardComponent.h
    cardComponent.cpp
    chooseCardComponent.h
    chooseCardComponent.cpp
    conditionComponent.h
    conditionComponent.cpp
    conditionImplComponent.h
    conditionImplComponent.cpp
    costComponent.h
    costComponent.cpp
    dbControls.h
    dbControls.cpp
    effectComponent.h
    effectComponent.cpp
    effectImplComponent.h
    effectImplComponent.cpp
    main.cpp
    multiplierComponent.h
    multiplierComponent.cpp
    multiplierImplComponent.h
    multiplierImplComponent.cpp
    placeComponent.h
    placeComponent.cpp
    statusLine.h
    statusLine.cpp
    targetComponent.h
    targetComponent.cpp
    triggerComponent.h
    triggerComponent.cpp
    triggerImplComponent.h
    triggerImplComponent.cpp
    qml.qrc)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(AbilityMaker WIN32 ${abilityMakerSources})

target_compile_features(AbilityMaker PRIVATE cxx_std_20)
target_compile_definitions(AbilityMaker
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
if (UNIX)
  target_compile_options(AbilityMaker PRIVATE -fcoroutines)
endif (UNIX)

target_link_libraries(AbilityMaker
  PRIVATE Qt5::Core Qt5::Quick dbManager)

target_link_libraries(AbilityMaker PUBLIC wsamateurAbilities)

install(TARGETS AbilityMaker RUNTIME DESTINATION ./)

# Ensure the Qt runtime libraries are installed
if(WIN32)
    include(Windeployqt)
    windeployqt(AbilityMaker ./)
endif()

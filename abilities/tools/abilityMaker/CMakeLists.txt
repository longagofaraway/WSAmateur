set(abilityMakerSources
    abilityComponent.h
    abilityComponent.cpp
    abilityMaker.h
    abilityMaker.cpp
    arrayOfAbilitiesComponent.h
    arrayOfAbilitiesComponent.cpp
    arrayOfEffectsComponent.h
    arrayOfEffectsComponent.cpp
    baseComponent.h
    baseComponent.cpp
    cardComponent.h
    cardComponent.cpp
    conditionComponent.h
    conditionComponent.cpp
    conditionImplComponent.h
    conditionImplComponent.cpp
    costComponent.h
    costComponent.cpp
    effectComponent.h
    effectComponent.cpp
    effectImplComponent.h
    effectImplComponent.cpp
    main.cpp
    placeComponent.h
    placeComponent.cpp
    targetComponent.h
    targetComponent.cpp
    triggerComponent.h
    triggerComponent.cpp
    triggerImplComponent.h
    triggerImplComponent.cpp
    qml.qrc)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(AbilityMaker ${abilityMakerSources})

target_compile_features(AbilityMaker PRIVATE cxx_std_20)
target_compile_definitions(AbilityMaker
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
if (UNIX)
  target_compile_options(AbilityMaker PRIVATE -fcoroutines)
endif (UNIX)

target_link_libraries(AbilityMaker
  PRIVATE Qt5::Core Qt5::Quick)

target_link_libraries(AbilityMaker PUBLIC wsamateurAbilities)

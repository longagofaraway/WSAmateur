set(abilityMakerSources
    abilityMaker.h
    abilityMaker.cpp
    main.cpp
    qml.qrc)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(AbilityMaker ${abilityMakerSources})

target_compile_features(AbilityMaker PRIVATE cxx_std_17)
target_compile_definitions(AbilityMaker
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(AbilityMaker
  PRIVATE Qt5::Core Qt5::Quick)

target_link_libraries(AbilityMaker PUBLIC wsamateurAbilities)

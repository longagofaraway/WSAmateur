set(abilityMakerSources
    main.cpp
    qml.qrc)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(AbilityMaker2 WIN32 ${abilityMakerSources})

target_compile_features(AbilityMaker2 PRIVATE cxx_std_20)
target_compile_definitions(AbilityMaker2
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
if (UNIX)
  target_compile_options(AbilityMaker2 PRIVATE -fcoroutines)
endif (UNIX)

target_link_libraries(AbilityMaker2
  PRIVATE Qt5::Core Qt5::Quick dbManager)

target_link_libraries(AbilityMaker2 PUBLIC wsamateurAbilities)

install(TARGETS AbilityMaker2 RUNTIME DESTINATION ./)

# Ensure the Qt runtime libraries are installed
if(WIN32)
    include(Windeployqt)
    windeployqt(AbilityMaker2 ./)
endif()

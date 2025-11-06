set(abilityMakerSources
    abilityMaker.h
    abilityMaker.cpp
    baseComponent.h
    baseComponent.cpp
    cardSpecifier.h
    cardSpecifier.cpp
    componentHelper.h
    componentHelper.cpp
    componentManager.h
    componentManager.cpp
    effect.h
    effect.cpp
    effectInit.h
    effectInit.cpp
    effectsTree.h
    effectsTree.cpp
    languageSpecification.h
    languageSpecification.cpp
    main.cpp
    target.h
    target.cpp
    trigger.h
    trigger.cpp
    triggerInit.h
    triggerInit.cpp
    qml.qrc)

set(langSpecSource
    effect.txt
    trigger.txt
    card.txt
    basicTypes.txt)

list(TRANSFORM langSpecSource PREPEND ${PROJECT_SOURCE_DIR}/docs/languageSpecification/)

add_custom_command(
    OUTPUT lang_spec.h
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/copy_lang_spec.py ${langSpecSource}
    DEPENDS ${langSpecSource} copy_lang_spec.py
    VERBATIM)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(AbilityMaker2 WIN32 ${abilityMakerSources} lang_spec.h)

target_include_directories(AbilityMaker2
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_features(AbilityMaker2 PRIVATE cxx_std_20)
target_compile_definitions(AbilityMaker2
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
if (UNIX)
  target_compile_options(AbilityMaker2 PRIVATE -fcoroutines)
endif (UNIX)

target_link_libraries(AbilityMaker2
  PRIVATE Qt5::Core Qt5::Quick dbManager languageParser)

target_link_libraries(AbilityMaker2 PUBLIC wsamateurAbilities)

install(TARGETS AbilityMaker2 RUNTIME DESTINATION ./)

# Ensure the Qt runtime libraries are installed
if(WIN32)
    include(Windeployqt)
    windeployqt(AbilityMaker2 ./)
endif()

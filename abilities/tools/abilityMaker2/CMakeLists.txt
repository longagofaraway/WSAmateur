set(abilityMakerSources
    ability.h
    ability.cpp
    abilityMaker.h
    abilityMaker.cpp
    baseComponent.h
    baseComponent.cpp
    card.h
    card.cpp
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
    condition.txt
    multiplier.txt
    basicTypes.txt)

set(helperFilesSource
    trigger.txt
    effect.txt)

list(TRANSFORM langSpecSource PREPEND ${PROJECT_SOURCE_DIR}/docs/languageSpecification/)
list(TRANSFORM helperFilesSource PREPEND ${PROJECT_SOURCE_DIR}/docs/languageSpecification/)

add_custom_command(
    OUTPUT lang_spec.h
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/copy_lang_spec.py ${langSpecSource}
    DEPENDS ${langSpecSource} copy_lang_spec.py
    VERBATIM)

add_custom_command(
    OUTPUT ability_maker_gen.cpp ability_maker_gen.h
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/generate_helper_files.py ${helperFilesSource}
    DEPENDS ${helperFilesSource} generate_helper_files.py
    VERBATIM)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(AbilityMaker2 WIN32 ${abilityMakerSources} lang_spec.h ability_maker_gen.cpp ability_maker_gen.h)

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

target_include_directories(AbilityMaker2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AbilityMaker2
  PRIVATE Qt5::Core Qt5::Quick dbManager languageParser)

target_link_libraries(AbilityMaker2 PUBLIC wsamateurAbilities)

install(TARGETS AbilityMaker2 RUNTIME DESTINATION ./)

# Ensure the Qt runtime libraries are installed
if(WIN32)
    include(Windeployqt)
    windeployqt(AbilityMaker2 ./)
endif()

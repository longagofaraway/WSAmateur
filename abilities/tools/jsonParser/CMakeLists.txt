set(jsonParserSources
    jsonParser.h
    jsonParser.cpp
    conditionParser.cpp
    effectParser.cpp
    abilityParser.cpp
    main.cpp
    qml.qrc)

set(languageSpec
    ability.txt
    cost.txt
    effect.txt
    trigger.txt
    condition.txt
    target.txt
    card.txt
    multiplier.txt
    basicTypes.txt)

list(TRANSFORM languageSpec PREPEND ${PROJECT_SOURCE_DIR}/docs/languageSpecification/)

add_custom_command(
    OUTPUT gen_serializer.cpp gen_serializer.h
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/ability_serializer.py ${languageSpec}
    DEPENDS ${languageSpec} ability_serializer.py
    VERBATIM)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(jsonParser WIN32 ${jsonParserSources} gen_serializer.cpp gen_serializer.h)

target_include_directories(jsonParser
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)
target_compile_features(jsonParser PRIVATE cxx_std_20)
target_compile_definitions(jsonParser
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(jsonParser
  PRIVATE Qt5::Core Qt5::Quick dbManager
  PUBLIC wsamateurAbilities)

install(TARGETS jsonParser RUNTIME DESTINATION ./)

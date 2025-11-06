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
    OUTPUT language_parser.h language_parser.cpp
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/language_parser.py ${languageSpec}
    DEPENDS ${languageSpec} language_parser.py
    VERBATIM)

add_library(languageParser STATIC language_parser.h language_parser.cpp)

target_compile_features(languageParser PRIVATE cxx_std_20)

target_include_directories(languageParser
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(languageParser PUBLIC wsamateurAbilities)

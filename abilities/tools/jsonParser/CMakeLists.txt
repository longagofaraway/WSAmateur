set(jsonParserSources
    jsonParser.h
    jsonParser.cpp
    conditionParser.cpp
    effectParser.cpp
    main.cpp
    qml.qrc)

find_package(Qt5 COMPONENTS Quick REQUIRED)

add_executable(jsonParser ${jsonParserSources})

target_compile_features(jsonParser PRIVATE cxx_std_20)
target_compile_definitions(jsonParser
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(jsonParser
  PRIVATE Qt5::Core Qt5::Quick)

target_link_libraries(jsonParser PUBLIC wsamateurAbilities)

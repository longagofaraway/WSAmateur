set(clientSources
    src/main.cpp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

add_executable(WSAmateur ${clientSources} qml.qrc)
target_compile_features(WSAmateur PRIVATE cxx_std_17)
target_compile_definitions(WSAmateur
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(WSAmateur
  PRIVATE Qt5::Core Qt5::Quick)

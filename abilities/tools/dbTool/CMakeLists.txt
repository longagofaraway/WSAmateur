set(dbToolSources
    dbTool.cpp)

find_package(Qt5 COMPONENTS Core REQUIRED)

add_executable(dbTool ${dbToolSources})

target_compile_features(dbTool PRIVATE cxx_std_20)
target_link_libraries(dbTool
  PRIVATE Qt5::Core dbManager wsamateurAbilities)

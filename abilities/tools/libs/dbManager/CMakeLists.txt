set(sources
    dbManager.h
    dbManager.cpp)

find_package(Qt5 COMPONENTS Sql REQUIRED)

add_library(dbManager STATIC ${sources})

target_compile_features(dbManager PRIVATE cxx_std_20)

target_include_directories(dbManager
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dbManager PUBLIC Qt5::Sql
  PRIVATE Qt5::Core)

target_link_libraries(dbManager PUBLIC wsamateurAbilities)

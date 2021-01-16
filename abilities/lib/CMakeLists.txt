set(commonSources
    abilities.h
    abilities/ability.h
    abilities/asnCard.h
    abilities/basicTypes.h
    abilities/condition.h
    abilities/cost.h
    abilities/effect.h
    abilities/number.h
    abilities/target.h
    abilities/trigger.h
    codecs/print.h
    codecs/printAbility.cpp
    codecs/printCard.cpp
    codecs/printCondition.cpp
    codecs/printEffect.cpp
    codecs/printTrigger.cpp
    codecs/printUtils.cpp
    )

add_library(wsamateurAbilities STATIC ${commonSources})

target_include_directories(wsamateurAbilities
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(wsamateurAbilities PRIVATE cxx_std_17)

#if (MSVC)
#    target_compile_options(wsamateurAbilities PRIVATE "/wd4251")
#endif()

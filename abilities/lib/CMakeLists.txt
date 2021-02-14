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
    codecs/decode.h
    codecs/decodeAbility.cpp
    codecs/decodeBasicTypes.cpp
    codecs/decodeCard.cpp
    codecs/decodeCondition.cpp
    codecs/decodeEffect.cpp
    codecs/decodeTrigger.cpp
    codecs/encDecUtils.h
    codecs/encode.h
    codecs/encodeAbility.cpp
    codecs/encodeBasicTypes.cpp
    codecs/encodeCard.cpp
    codecs/encodeCondition.cpp
    codecs/encodeEffect.cpp
    codecs/encodeTrigger.cpp
    codecs/print.h
    codecs/printAbility.cpp
    codecs/printCard.cpp
    codecs/printCondition.cpp
    codecs/printCost.cpp
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

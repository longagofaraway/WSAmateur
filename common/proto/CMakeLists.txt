set(protoSources
    abilities.proto
    attackType.proto
    cardAttribute.proto
    commandContainer.proto
    gameCommand.proto
    gameEvent.proto
    moveCommands.proto
    moveEvents.proto
    lobbyCommand.proto
    lobbyEvent.proto
    phaseCommand.proto
    phaseEvent.proto
    serverMessage.proto
    )

set (Protobuf_IMPORT_DIRS ${Protobuf_INCLUDE_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${protoSources})

set_property(SOURCE ${PROTO_SRCS} ${PROTO_HDRS} PROPERTY SKIP_AUTOGEN ON)

add_library(wsamateurProto STATIC ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(wsamateurProto
    INTERFACE
        ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC
        ${Protobuf_INCLUDE_DIRS}
)

target_compile_features(wsamateurProto PRIVATE cxx_std_20)

set(wsamateurProto_LIBS ${PROTOBUF_LIBRARIES})
if (UNIX)
    set(wsamateurProto_LIBS ${wsamateurProto_LIBS} -pthread)
endif (UNIX)

target_link_libraries(wsamateurProto PRIVATE ${wsamateurProto_LIBS})


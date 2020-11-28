set(commonSources
    connection.cpp
    serverProtocolHandler.cpp
    connection.h
    serverProtocolHandler.h
    )

include_directories(${Qt5Core_INCLUDE_DIRS})

add_library(wsamateurCommon STATIC ${commonSources})


set(severSources
    connectionPool.h
    networkConnectionManager.h
    tcpConnection.h
    tcpServer.h
    connectionPool.cpp
    main.cpp
    networkConnectionManager.cpp
    tcpConnection.cpp
    tcpServer.cpp
    )


find_package(Qt5 COMPONENTS Network REQUIRED)

add_executable(wsamateurServer ${severSources})

target_compile_features(wsamateurServer PRIVATE cxx_std_20)
if (UNIX)
        target_compile_options(wsamateurServer PUBLIC -fcoroutines)
endif (UNIX)

target_link_libraries(wsamateurServer PRIVATE wsamateurCore
    PRIVATE Qt5::Core Qt5::Network)

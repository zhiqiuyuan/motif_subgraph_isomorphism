set(UTIL_SRC
        GenerateTVEFileQ.cpp
        GenerateTVEFileQ.h
        GraphFeatures.cpp
        GraphFeatures.h
        tools.cpp
        tools.h
        types.cpp
        types.h)

add_library(util SHARED
        ${UTIL_SRC})

add_executable(turnEFile2TVEFile TurnEFile2TVEFile.cpp tools.cpp)
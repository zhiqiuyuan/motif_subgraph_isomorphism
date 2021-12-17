set(UTIL_SRC
        GenerateTVEFileQ.cpp
        GenerateTVEFileQ.h
        GraphFeatures.cpp
        GraphFeatures.h
        tools.cpp
        tools.h
        types.h)

add_library(util SHARED
        ${UTIL_SRC})
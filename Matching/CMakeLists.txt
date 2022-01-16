set(MATCHING_SRC
        Filter.cpp
        Filter.h
        GenerateOrder.cpp
        GenerateOrder.h)

add_library(Matching SHARED
        ${MATCHING_SRC})
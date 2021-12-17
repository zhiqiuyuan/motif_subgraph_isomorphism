set(MOTIFGRAPH_SRC
        Graph.cpp
        Graph.h
        Motif.cpp
        Motif.h
        MotifG.cpp
        MotifG.h
        MotifQ.cpp
        MotifQ.h)

add_library(MotifGraph SHARED
        ${MOTIFGRAPH_SRC})
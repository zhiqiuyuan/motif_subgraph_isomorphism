set(FILTERWRITE_CANDISCALE_FILE_LIST
        FilterWriteCandiScale.cpp
        TVEFileG.cpp
        TVEFileG.h
        TVEFileQ.cpp
        TVEFileQ.h)

set(OFFLINE_FILE_LIST
        Offline.cpp
        TVEFileG.cpp
        TVEFileG.h)

set(GENERATETVEFILEQ_FILE_LIST
        GenerateTVEFileQ.cpp
        TVEFileG.cpp
        TVEFileG.h
)

set(COLLECTGGRAPHFEATURES_FILE_LIST
        CollectGGraphFeatures.cpp
        TVEFileG.cpp
        TVEFileG.h
)

set(COLLECTQGRAPHFEATURES_FILE_LIST
        CollectQGraphFeatures.cpp
        TVEFileG.cpp
        TVEFileG.h
        TVEFileQ.cpp
        TVEFileQ.h
)

add_executable(filterWriteCandiScale ${FILTERWRITE_CANDISCALE_FILE_LIST})
target_link_libraries(filterWriteCandiScale util MotifGraph Matching bulkQueries)

add_executable(offline ${OFFLINE_FILE_LIST})
target_link_libraries(offline util MotifGraph)

add_executable(generateTVEFileQ ${GENERATETVEFILEQ_FILE_LIST})
target_link_libraries(generateTVEFileQ util MotifGraph)

add_executable(collectGGraphFeatures ${COLLECTGGRAPHFEATURES_FILE_LIST})
target_link_libraries(collectGGraphFeatures util MotifGraph)

add_executable(collectQGraphFeatures ${COLLECTQGRAPHFEATURES_FILE_LIST})
target_link_libraries(collectQGraphFeatures util MotifGraph Matching bulkQueries)

#add_executable(EdgeListConverter.out EdgeListToCSR.cpp)
#add_executable(ReassignLabel ReassignLabel.cpp)

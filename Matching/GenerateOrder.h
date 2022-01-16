#ifndef _GENERATEORDER_H
#define _GENERATEORDER_H

#include "../util/types.h"
#include "../util/GraphFeatures.h"
#include "../MotifGraph/Graph.h"
#include "Filter.h"

class GenerateOrder
{
private:
public:
    GenerateOrder();
    ~GenerateOrder();

    static VertexID selectCFLFilterStartVertex(const Graph *data_graph, const Graph *query_graph);

    static void generateCFLFilterOrder(const Graph *data_graph, const Graph *query_graph, TreeNode *&tree,
                                       VertexID *&order, int &level_count, unsigned *&level_offset);
    static VertexID selectDPisoFilterStartVertex(const Graph *data_graph, const Graph *query_graph);

    static void generateDPisoFilterOrder(const Graph *data_graph, const Graph *query_graph, TreeNode *&tree,
                                         VertexID *&order);
};

#endif //_GENERATEORDER_H
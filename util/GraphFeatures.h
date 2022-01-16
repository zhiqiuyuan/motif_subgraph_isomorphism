#ifndef _GRAPHFEATURES_H
#define _GRAPHFEATURES_H

#include "types.h"
#include "../MotifGraph/Graph.h"

class Graph;

class GraphFeatures
{
    /*提供对成员Graph*graph输出features的方法
    */
private:
    Graph *graph;

public:
    GraphFeatures(Graph *g);
    virtual ~GraphFeatures();

    /* 需要graph已经加载基础结构
    */
    unsigned getTotalBidegree();

    /* WARNNING: 这样求出的out_children_是不包含bi_children的，而in_children_是包含的，且没有写生成bi_children_是逻辑
    下一个邻居为所有邻居，不只是out 
    */
    void weakBFS(VertexID root_vertex, TreeNode *&tree, VertexID *&bfs_order);                            //对this->graph
    static void weakBFS(const Graph *graph, VertexID root_vertex, TreeNode *&tree, VertexID *&bfs_order); //对传入的graph                                     //对传入的graph
    /* 当无向图处理 */
    static void getKCore(const Graph *graph, int *core_table);

    /* 检查弱联通性，从root_vertex顶点出发,BFS */
    static bool isWeakConnectedFromRoot(const Graph *graph, VertexID root_vertex);
};

#endif //_GRAPHFEATURES_H
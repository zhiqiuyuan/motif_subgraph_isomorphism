#include "types.h"
#include "../MotifGraph/Graph.h"

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
};

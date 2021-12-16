#include "../config/config.h"
#include "../MotifGraph/Graph.h"
#include "tools.h"

//for generate query graph
class QGraph
{
public:
    int curr_vcnt;
    bool isSparse;                   //1 average_degree<=3
    std::vector<unsigned> edge_from; //vertexID->edge_from
    std::vector<unsigned> edge_to;
    std::set<unsigned> vertices;
    std::map<unsigned, int> id2degree; //indegree+outdegree
    QGraph(int isSparse0) : isSparse(isSparse0)
    {
        curr_vcnt = 0;
    }
    int getVerticesCount()
    {
        return curr_vcnt;
    }
    int getEdgesCount()
    {
        return edge_from.size();
    }
    int getTotalDegree()
    {
        int re = 0;
        for (auto item : id2degree)
        {
            re += item.second;
        }
        return re;
    }
    int getVertexDegree(unsigned vid)
    {
        return id2degree[vid];
    }
    double getAverageDegree()
    {
        int total = getTotalDegree();
        return ((double)total) / getVerticesCount();
    }

    /* return whether remove successfully
    * one degree turn 0 would restore degree updates, i.e. removes failed
    */
    bool try_remove_1edge_update_degree(unsigned remove_edge_idx)
    {
        unsigned fromId = edge_from[remove_edge_idx], toId = edge_to[remove_edge_idx];
        if (id2degree[fromId] <= 1 || id2degree[toId] <= 1)
        {
            return 0;
        }
        id2degree[fromId]--;
        id2degree[toId]--;
        return 1;
    }
    bool addVertex(unsigned vid, const Graph *dg)
    {
        if (std::find(vertices.begin(), vertices.end(), vid) != vertices.end())
        {
            return 0; //vid already in
        }
        curr_vcnt++;
        vertices.insert(vid);
        id2degree[vid] = 0;

        //add all edges connecting vid and vertices
        for (unsigned v : vertices)
        {
            if (dg->checkEdgeExistence(v, vid))
            {
                id2degree[v]++;
                id2degree[vid]++;
                edge_from.push_back(v);
                edge_to.push_back(vid);
            }
            if (dg->checkEdgeExistence(vid, v))
            {
                id2degree[v]++;
                id2degree[vid]++;
                edge_from.push_back(vid);
                edge_to.push_back(v);
            }
        }
    }
    bool isVertex(unsigned vid)
    {
        return vertices.count(vid);
    }
    void copyVerticesToVector(std::vector<unsigned> &dest)
    {
        dest.clear();
        for (unsigned id : vertices)
        {
            dest.push_back(id);
        }
    }
    bool allVerticesAlreadyIn(const unsigned *arr, unsigned arr_sz)
    {
        for (unsigned i = 0; i < arr_sz; ++i)
        {
            if (isVertex(arr[i]) == 0)
            {
                return 0;
            }
        }
        return 1;
    }
};

/* dfs from start
 * curr_qcnt:for file name
 * dest_dir: write to dest_dir/query_dense_vcnt_currqcnt.graph
 * file format: t v e
 * return whether found one
 * degree_threshold: some data_graph are too sparse that cannot find dense subgraph whose average degree>=3, some adjust it
 */
bool gen_qgraph_from_start(unsigned start, unsigned vcnt_required, unsigned curr_qcnt, unsigned degree_threshold, std::string dest_dir, QGraph *currq, Graph *dg);

/* sparse_arr: {0} {1} {0,1}
 * dest_dir: write to dest_dir/query_dense_vcnt_currqcnt.graph  xxx/youtube/query_graph/
 * file format: t v e
 * when generating dense query_graph, it's possible to abort (when the function finds the data_graph is too sparse)
 */
void gen_qgraph(Graph *dg, std::vector<unsigned> vcnt_arr, std::vector<bool> sparse_arr, unsigned qcnt_required, std::string dest_dir, int GAP);
#include "config.h"
#include "types.h"
#include "Graph.h"
#include <time.h>

//for generate query graph
struct QGraph
{
    int curr_vcnt;
    bool isSparse;             //1 average_degree<=3
    std::vector<ui> edge_from; //vertexID->edge_from
    std::vector<ui> edge_to;
    std::set<ui> vertices;
    std::map<ui, int> id2degree; //indegree+outdegree
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
    int getVertexDegree(ui vid)
    {
        return id2degree[vid];
    }
    double getAverageDegree()
    {
        int total = getTotalDegree();
        return ((double)total) / getVerticesCount();
    }
    //return whether remove successfully
    //one degree turn 0 would restore degree updates, i.e. removes failed
    bool try_remove_1edge_update_degree(ui remove_edge_idx)
    {
        ui fromId = edge_from[remove_edge_idx], toId = edge_to[remove_edge_idx];
        if (id2degree[fromId] <= 1 || id2degree[toId] <= 1)
        {
            return 0;
        }
        id2degree[fromId]--;
        id2degree[toId]--;
        return 1;
    }
    bool addVertex(ui vid, const Graph *dg)
    {
        if (std::find(vertices.begin(), vertices.end(), vid) != vertices.end())
        {
            return 0; //vid already in
        }
        curr_vcnt++;
        vertices.insert(vid);
        id2degree[vid] = 0;

        //add all edges connecting vid and vertices
        for (ui v : vertices)
        {
            if (dg->checkEdgeExistence_strict(v, vid))
            {
                id2degree[v]++;
                id2degree[vid]++;
                edge_from.push_back(v);
                edge_to.push_back(vid);
            }
            if (dg->checkEdgeExistence_strict(vid, v))
            {
                id2degree[v]++;
                id2degree[vid]++;
                edge_from.push_back(vid);
                edge_to.push_back(v);
            }
        }
    }
    bool isVertex(ui vid)
    {
        return vertices.count(vid);
    }
    void copyVerticesToVector(std::vector<ui> &dest)
    {
        dest.clear();
        for (ui id : vertices)
        {
            dest.push_back(id);
        }
    }
    bool allVerticesAlreadyIn(const ui *arr, ui arr_sz)
    {
        for (ui i = 0; i < arr_sz; ++i)
        {
            if (isVertex(arr[i]) == 0)
            {
                return 0;
            }
        }
        return 1;
    }
};

//return [0,possible_max)
long long Rand(long long possible_max);

//concate as left+right: pick element from left+right at random
ui getRandFrom2Array(const ui *left, const ui *right, ui left_sz, ui right_sz);

//cnt: num of values included in old_ave
//???cnt???0?new_val???????old_ave ???????0?
void moving_average(double &old_ave, double new_val, ui &cnt);

//dfs from start
//curr_qcnt:for file name
//dest_dir: write to dest_dir/query_dense_vcnt_currqcnt.graph
//return whether found one
//degree_threshold: some data_graph are too sparse that cannot find dense subgraph whose average degree>=3, some adjust it
bool gen_qgraph_from_start(ui start, ui vcnt_required, ui curr_qcnt, ui degree_threshold, std::string dest_dir, QGraph *currq, Graph *dg);

//sparse_arr: {0} {1} {0,1}
//dest_dir: write to dest_dir/query_dense_vcnt_currqcnt.graph
void gen_qgraph(Graph *dg, std::vector<ui> vcnt_arr, std::vector<bool> sparse_arr, ui qcnt_required, std::string dest_dir);
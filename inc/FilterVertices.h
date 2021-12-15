#ifndef FILTERVERTICES_H
#define FILTERVERTICES_H

#include "Graph.h"
#include <memory.h>
#include <vector>
#include <algorithm>
#include <fstream>
#define INVALID_VERTEX_ID 100000000

class FilterVertices
{
public:
    static bool LDFFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);
    static bool NLFFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);
    static bool NLFFilter_1step(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);

#if TOPO_MOTIF_ENABLE == 1
    /*
    1.topo only, label not considered
    2.data_graph,query_graph: motif structure already loaded
    3.will allocate space for candidates and candidates_count
    */
    static bool MotifFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count, bool use_nlf);
    static bool MotifFilter_1step(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count, bool use_nlf);

#endif //TOPO_MOTIF_ENABLE == 1
#if LABEL_MOTIF_ENABLE == 1
    /*
    1.vertex label considered
    2.data_graph,query_graph: motif structure already loaded
    3.will allocate space for candidates and candidates_count
    */
    static bool LabelMotifFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);
    static bool LabelMotifFilter_1step(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);
#if LABEL_MOTIF_LIMIT == 1
    static bool LabelMotifFilter_limit(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);
#endif //LABEL_MOTIF_LIMIT==1
    /*
    feature:[q_v特征中在G中找到的数目,q_v的总特征数,
    记录为可以覆盖的q_v当中覆盖的q_v的特征,记录为可以覆盖的q_v当中q_v的总特征数]
    为统计时间服务
    */
    static bool LabelMotifFilter_collect_data_feature(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count, ui *feature);

#endif //LABEL_MOTIF_ENABLE == 1

private:
#if DIRECTED_GRAPH == 1
    static void computeCandidateWithNLF_directed(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                                 ui &count, ui *buffer = NULL);
#else
    static void computeCandidateWithNLF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                        ui &count, ui *buffer);
#endif //DIRECTED_GRAPH==1
    static void compactCandidates(ui **&candidates, ui *&candidates_count, ui query_vertex_num);
    static bool isCandidateSetValid(ui **&candidates, ui *&candidates_count, ui query_vertex_num);
    static void allocateBuffer(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count);

    //return darr covers qarr in first sz dime
    static bool arr_cover(const ui *darr, const ui *qarr, ui sz);
    //return dmap covers qmap
    //cover: key kind covers; for the same key, cnt covers
    static bool map_cover(const std::unordered_map<LabelID, ui> *dmap, const std::unordered_map<LabelID, ui> *qmap);
    //return g_lmtf_arr(size k) covers q_lmtf_arr(size q_lmtf_arr_sz)
    //cover: for every entry in q_lmtf_arr, if exist in g_lmtf_arr, must be coverd
    static bool lmtf_arr_cover(const Lmtf *g_lmtf_arr, ui k, const Lmtf *q_lmtf_arr, ui q_lmtf_arr_sz);
    static bool lmtf_arr_cover_collect_data_feature(const Lmtf *g_lmtf_arr, ui k, const Lmtf *q_lmtf_arr, ui q_lmtf_arr_sz, ui *feature);

    //return for every q_v_n in q_in_neighbors, there exits a d_v_n in d_in_neighbors that covers q_v_n
    //cover: ldf+nlf
    static bool check_nlf_cover(ui in_count, ui d_in_count, const ui *q_in_neighbors, const ui *d_in_neighbors, const Graph *query_graph, const Graph *data_graph);
    //cover: ldf+nlf+lmtf
    static bool check_lmtf_cover(ui in_count, ui d_in_count, const ui *q_in_neighbors, const ui *d_in_neighbors, const Graph *query_graph, const Graph *data_graph);

    static ui bi_search(Lmtf *q_lmtf_arr, ui ub, ui ue, ui code, int &new_e);
    static void print_candi(ui **candidates, ui *candidates_count, ui vertex_num);
};
#endif // FILTERVERTICES_H

#ifndef _FILTER_H
#define _FILTER_H

#include "../MotifGraph/MotifG.h"
#include "../MotifGraph/MotifQ.h"
#include "../util/tools.h"
#include "GenerateOrder.h"

#define INVALID_VERTEX_ID 100000000

class Filter
{
public:
    /* WARNNING: 下面所有一步过滤的实现都有问题：都不是用已经筛选出的候选解进行，而是对一阶邻居再次应用筛选定义去计算
*/
    static bool LDFFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count);

    static bool NLFFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count);
    static bool NLFFilter1step(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count);

    static bool GQLFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count);
    static bool CFLFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count,
                          unsigned *&order, TreeNode *&tree);
    static bool DPisoFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count,
                            unsigned *&order, TreeNode *&tree);
#if TOPO_MOTIF_ENABLE == 1
    /*
    1.topo only, label not considered
    2.data_graph,query_graph: motif structure already loaded
    3.will allocate space for candidates and candidates_count
    */
    static bool TopoMotifFilter(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, bool use_nlf);
    static bool TopoMotifFilter1step(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, bool use_nlf);

#endif //TOPO_MOTIF_ENABLE == 1
#if LABEL_MOTIF_ENABLE == 1
    /* WARNNING:此函数实现的逻辑为数据图固定存k维，已经注释掉，即此函数尚未实现
    1.vertex label considered
    2.data_graph,query_graph: motif structure already loaded
    3.data_graph: k already loaded
    4.will allocate space for candidates and candidates_count
    */
    static bool LabelMotifFilter(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, std::string firstStageFilterKind = "");
    static bool LabelMotifFilter1step(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count);
#if LABEL_MOTIF_LIMIT == 1
    static bool LabelMotifFilter_limit(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, std::string firstStageFilterKind = "");
#endif //LABEL_MOTIF_LIMIT==1
    /*
    feature:[q_v特征中在G中找到的数目,q_v的总特征数,
    记录为可以覆盖的q_v当中覆盖的q_v的特征,记录为可以覆盖的q_v当中q_v的总特征数]
    为统计时间服务
    */
    static bool LabelMotifFilter_collect_data_feature(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, unsigned *feature);

    /* AVERAGE CANDISCALE
* 返回平均候选解规模（对于查询图的每个顶点做平均），返回负值表示失败
* 传入的数据图需要加载好基本结构和相应过滤方法的离线结构
* 传入的查询图需要加载好基本结构（查询图的对应数据图离线结构部分在此函数中在线计算）
* filename: xxx/query_isSparse_qVScale_j.graph，用于WRITE_TO_FILE_DEBUG在线时把查询图motif结构的计算结果写入文件
*/
    //数据图和查询图加载基本结构
    static double LDFFilter_AveCandiScale(const Graph *data_graph, const Graph *query_graph);

    static double NLFFilter_AveCandiScale(const Graph *data_graph, Graph *query_graph);
    static double NLFFilter1step_AveCandiScale(const Graph *data_graph, Graph *query_graph);

    static double GQLFilter_AveCandiScale(const Graph *data_graph, Graph *query_graph);
    static double CFLFilter_AveCandiScale(const Graph *data_graph, Graph *query_graph);
    static double DPisoFilter_AveCandiScale(const Graph *data_graph, Graph *query_graph);
#if TOPO_MOTIF_ENABLE == 1
    /* WARNNING: 下述TopoMotif实现为硬编码使用NLF，可以在实现中传递给TopoMotifFilter的参数修改
*/
    static double TopoMotifFilter_AveCandiScale(const MotifG *data_graph, MotifQ *query_graph, std::string filename = "");
    static double TopoMotifFilter1step_AveCandiScale(const MotifG *data_graph, MotifQ *query_graph, std::string filename = "");
#endif //#if TOPO_MOTIF_ENABLE == 1
/* WARNNING: 没有实现普通labelMotifFilter的这个函数
*/
#if LABEL_MOTIF_ENABLE == 1
#if LABEL_MOTIF_LIMIT == 1
    static double LabelMotifFilter_limit_AveCandiScale(const MotifG *data_graph, MotifQ *query_graph, std::string firstStageFilterKind = "", std::string filename = "");
#endif //#if LABEL_MOTIF_LIMIT == 1
#endif //#if LABEL_MOTIF_ENABLE == 1

#endif //LABEL_MOTIF_ENABLE == 1

    static void computeCandidateWithNLF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                        unsigned &count, unsigned *buffer = NULL);
    static void computeCandidateWithLDF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                        unsigned &count, unsigned *buffer = NULL);

private:
    //return darr covers qarr in first sz dime
    static bool arr_cover(const unsigned *darr, const unsigned *qarr, unsigned sz);
    //return dmap covers qmap
    //cover: key kind covers; for the same key, cnt covers
    static bool map_cover(const std::unordered_map<LabelID, unsigned> *dmap, const std::unordered_map<LabelID, unsigned> *qmap);
    //return g_lmtf_arr(size k) covers q_lmtf_arr(size q_lmtf_arr_sz)
    //cover: for every entry in q_lmtf_arr, if exist in g_lmtf_arr, must be coverd
    static bool lmtf_arr_cover(const Lmtf *g_lmtf_arr, unsigned k, const Lmtf *q_lmtf_arr, unsigned q_lmtf_arr_sz);
    static bool lmtf_arr_cover_collect_data_feature(const Lmtf *g_lmtf_arr, unsigned k, const Lmtf *q_lmtf_arr, unsigned q_lmtf_arr_sz, unsigned *feature);

    //return for every q_v_n in q_in_neighbors, there exits a d_v_n in d_in_neighbors that covers q_v_n
    //cover: ldf+nlf
    static bool check_nlf_cover(unsigned in_count, unsigned d_in_count, const unsigned *q_in_neighbors, const unsigned *d_in_neighbors, const Graph *query_graph, const Graph *data_graph);
    //cover: ldf+nlf+lmtf
    static bool check_lmtf_cover(unsigned in_count, unsigned d_in_count, const unsigned *q_in_neighbors, const unsigned *d_in_neighbors, const MotifQ *query_graph, const MotifG *data_graph);

    static unsigned bi_search(Lmtf *q_lmtf_arr, unsigned ub, unsigned ue, unsigned code, int &new_e);
    static void old_cheap(int *col_ptrs, int *col_ids, int *match, int *row_match, int n, int m);
    static void match_bfs(int *col_ptrs, int *col_ids, int *match, int *row_match, int *visited,
                          int *queue, int *previous, int n, int m);
    static bool checkInjection(const unsigned *query_vertex_neighbors, const unsigned *data_vertex_neighbors, unsigned left_partition_size, unsigned right_partition_size,
                               bool **valid_candidates, int *left_to_right_offset, int *left_to_right_edges,
                               int *left_to_right_match, int *right_to_left_match, int *match_visited,
                               int *match_queue, int *match_previous);

    static void compactCandidates(unsigned **&candidates, unsigned *&candidates_count, unsigned query_vertex_num);
    static bool isCandidateSetValid(unsigned **&candidates, unsigned *&candidates_count, unsigned query_vertex_num);
    static void allocateBuffer(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count);
    static void print_candi(unsigned **candidates, unsigned *candidates_count, unsigned vertex_num);

    /* 
    所有已经求出候选集的 
    1.in邻居候选集的out邻居
    2.out邻居候选集的in邻居
    3.bi邻居候选集的bi邻居
    的交集

    pivot_kind: bn, fn, under_level
    */
    static void generateCandidates(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                   TreeNode &node, std::string pivot_kind, VertexID **candidates,
                                   unsigned *candidates_count, unsigned *flag, unsigned *updated_flag);

    static void pruneCandidates(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                TreeNode &node, std::string pivot_kind, VertexID **candidates,
                                unsigned *candidates_count, unsigned *flag, unsigned *updated_flag);
};
#endif // _FILTER_H

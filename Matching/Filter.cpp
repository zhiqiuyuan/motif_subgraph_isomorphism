#include "Filter.h"
bool Filter::LDFFilter(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count)
{
    allocateBuffer(data_graph, query_graph, candidates, candidates_count);

    for (unsigned i = 0; i < query_graph->getVerticesCount(); ++i)
    {
        LabelID label = query_graph->getVertexLabel(i);

        unsigned data_vertex_num;
        const unsigned *data_vertices = data_graph->getVerticesByLabel(label, data_vertex_num);
        unsigned indegree = query_graph->getVertexInDegree(i);
        unsigned outdegree = query_graph->getVertexOutDegree(i);
        unsigned bidegree = query_graph->getVertexBiDegree(i);
        for (unsigned j = 0; j < data_vertex_num; ++j)
        {
            unsigned data_vertex = data_vertices[j];
            if ((data_graph->getVertexInDegree(data_vertex) >= indegree) && (data_graph->getVertexOutDegree(data_vertex) >= outdegree) && (data_graph->getVertexBiDegree(data_vertex) >= bidegree))
            {
                candidates[i][candidates_count[i]++] = data_vertex;
            }
        }

        if (candidates_count[i] == 0)
        {
            return false;
        }
    }

    return true;
}

bool Filter::check_nlf_cover(unsigned in_count, unsigned d_in_count, const unsigned *q_in_neighbors, const unsigned *d_in_neighbors, const MotifQ *query_graph, const MotifG *data_graph)
{
    for (unsigned ii = 0; ii < in_count; ++ii) //for all q_v_n, find covers in d_v_n
    {
        unsigned q_v_n = q_in_neighbors[ii];
        const std::unordered_map<LabelID, unsigned> *q_in_map = query_graph->getVertexInNLF(q_v_n);
        const std::unordered_map<LabelID, unsigned> *q_out_map = query_graph->getVertexOutNLF(q_v_n);
        const std::unordered_map<LabelID, unsigned> *q_bi_map = query_graph->getVertexBiNLF(q_v_n);
        unsigned q_in_d = query_graph->getVertexInDegree(q_v_n);
        unsigned q_out_d = query_graph->getVertexOutDegree(q_v_n);
        unsigned q_bi_d = query_graph->getVertexBiDegree(q_v_n);
        LabelID q_v_n_label = query_graph->getVertexLabel(q_v_n);

        unsigned jj;
        for (jj = 0; jj < d_in_count; ++jj)
        {
            unsigned d_v_n = d_in_neighbors[jj];
            if (data_graph->getVertexLabel(d_v_n) == q_v_n_label)
            {
#if STEP_DEBUG == 1
                std::cout << "q_v_n:" << q_v_n << "\nd_v_n:" << d_v_n << std::endl;
#endif //STEP_DEBUG==1
                if (((data_graph->getVertexInDegree(d_v_n)) >= q_in_d) && ((data_graph->getVertexOutDegree(d_v_n)) >= q_out_d) && ((data_graph->getVertexBiDegree(d_v_n)) >= q_bi_d))
                {

                    const std::unordered_map<LabelID, unsigned> *d_in_map = data_graph->getVertexInNLF(d_v_n);
                    const std::unordered_map<LabelID, unsigned> *d_out_map = data_graph->getVertexOutNLF(d_v_n);
                    const std::unordered_map<LabelID, unsigned> *d_bi_map = data_graph->getVertexBiNLF(d_v_n);
                    if (map_cover(d_in_map, q_in_map) && map_cover(d_out_map, q_out_map) && map_cover(d_bi_map, q_bi_map))
                    {
                        break; //q_v_n find a cover in d_v_n
                    }
                }
            }
        }
        if (jj >= d_in_count)
        {
            return 0;
        }
    }
    return 1;
}

#if TOPO_MOTIF_ENABLE == 1
/*
1.topo only, label not considered
2.data_graph,query_graph: motif structure already loaded
3.will allocate space for candidates and candidates_count
*/
bool Filter::TopoMotifFilter(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, bool use_nlf)
{
    if (use_nlf)
    {
        if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //NLF
            return false;
    }
    else
    {
        if (!LDFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF
            return false;
    }
#if STEP_DEBUG == 1
    print_candi(candidates, candidates_count, query_graph->getVerticesCount());
#endif //#if STEP_DEBUG == 1
    unsigned query_vertex_num = query_graph->getVerticesCount();
    unsigned data_vertex_num = data_graph->getVerticesCount();

    /*
    motif filter
    */
    VertexID *q_candidates;
    unsigned q_candidates_num;
    //motif_count_
    unsigned *q_motif_count, *g_motif_count;
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        //VertexID query_vertex = i;
        q_motif_count = query_graph->getVertexMotifCount(i);
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        for (unsigned j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                g_motif_count = data_graph->getVertexMotifCount(data_vertex);
                for (unsigned k = 0; k < MOTIF_COUNT_DEMENSION; ++k)
                {
                    if (g_motif_count[k] < q_motif_count[k])
                    {
                        q_candidates[j] = INVALID_VERTEX_ID;
                    }
                }
            }
        }
    }

    /*
    Compact candidates
    */
    compactCandidates(candidates, candidates_count, query_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, query_vertex_num);
}

/*
1.MotifFilter first, then one step filtering according to topo motif count
*/
bool Filter::TopoMotifFilter1step(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, bool use_nlf)
{
    if (!TopoMotifFilter(data_graph, query_graph, candidates, candidates_count, use_nlf))
    {
        //ldf + topo motif filter only on this, without considering neighbors
        return false;
    }

    unsigned query_vertex_num = query_graph->getVerticesCount();
    unsigned data_vertex_num = data_graph->getVerticesCount();

    /*
    one step motif filter
    */
    VertexID *q_candidates;
    unsigned q_candidates_num;
    //motif_count_
    unsigned *q_motif_count, *d_motif_count;
    const unsigned *q_in_neighbors, *q_out_neighbors, *q_bi_neighbors;
    unsigned q_in_neighbors_num, q_out_neighbors_num, q_bi_neighbors_num;
    unsigned q_neighbor_vertex, d_neighbor_vertex;
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        VertexID query_vertex = i;
        q_candidates = candidates[query_vertex];
        q_candidates_num = candidates_count[query_vertex];

        q_in_neighbors = query_graph->getVertexInNeighbors(query_vertex, q_in_neighbors_num);
        q_out_neighbors = query_graph->getVertexOutNeighbors(query_vertex, q_out_neighbors_num);
        q_bi_neighbors = query_graph->getVertexBiNeighbors(query_vertex, q_bi_neighbors_num);

        for (unsigned j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                unsigned d_in_count, d_out_count, d_bi_count;
                const unsigned *d_in_neighbors = data_graph->getVertexInNeighbors(data_vertex, d_in_count);
                const unsigned *d_out_neighbors = data_graph->getVertexOutNeighbors(data_vertex, d_out_count);
                const unsigned *d_bi_neighbors = data_graph->getVertexBiNeighbors(data_vertex, d_bi_count);
                if (check_nlf_cover(q_in_neighbors_num, d_in_count, q_in_neighbors, d_in_neighbors, query_graph, data_graph) && check_nlf_cover(q_out_neighbors_num, d_out_count, q_out_neighbors, d_out_neighbors, query_graph, data_graph) && check_nlf_cover(q_bi_neighbors_num, d_bi_count, q_bi_neighbors, d_bi_neighbors, query_graph, data_graph))
                {
                    unsigned d_neighbors_num;
                    //in
                    const unsigned *d_neighbors = data_graph->getVertexInNeighbors(data_vertex, d_neighbors_num);
                    //in_neighbors of data_vertex need to cover every in_neighbor of query_vertex
                    for (unsigned ii = 0; ii < q_in_neighbors_num; ++ii)
                    {
                        q_neighbor_vertex = q_in_neighbors[ii];
                        q_motif_count = query_graph->getVertexMotifCount(q_neighbor_vertex);
                        unsigned jj;
                        for (jj = 0; jj < d_neighbors_num; ++jj)
                        {
                            d_neighbor_vertex = d_neighbors[jj];
                            d_motif_count = data_graph->getVertexMotifCount(d_neighbor_vertex);
                            if (arr_cover(d_motif_count, q_motif_count, MOTIF_COUNT_DEMENSION))
                            { //d_neighbor_vertex covers q_neighbor_vertex
                                break;
                            }
                        }
                        if (jj >= d_neighbors_num)
                        { //q_neighbor_vertex didn't find any cover in d_neighbors
                            q_candidates[j] = INVALID_VERTEX_ID;
                            data_vertex = INVALID_VERTEX_ID;
                            break;
                        }
                    }

                    if (data_vertex != INVALID_VERTEX_ID)
                    {
                        //passed in_neighbors 1step check
                        //out
                        d_neighbors = data_graph->getVertexOutNeighbors(data_vertex, d_neighbors_num);
                        for (unsigned ii = 0; ii < q_out_neighbors_num; ++ii)
                        {
                            q_neighbor_vertex = q_out_neighbors[ii];
                            q_motif_count = query_graph->getVertexMotifCount(q_neighbor_vertex);
                            unsigned jj;
                            for (jj = 0; jj < d_neighbors_num; ++jj)
                            {
                                d_neighbor_vertex = d_neighbors[jj];
                                d_motif_count = data_graph->getVertexMotifCount(d_neighbor_vertex);
                                if (arr_cover(d_motif_count, q_motif_count, MOTIF_COUNT_DEMENSION))
                                { //d_neighbor_vertex covers q_neighbor_vertex
                                    break;
                                }
                            }
                            if (jj >= d_neighbors_num)
                            { //q_neighbor_vertex didn't find any cover in d_neighbors
                                q_candidates[j] = INVALID_VERTEX_ID;
                                data_vertex = INVALID_VERTEX_ID;
                                break;
                            }
                        }

                        if (data_vertex != INVALID_VERTEX_ID)
                        {
                            //passed in_out_neighbors 1step check
                            //bi
                            d_neighbors = data_graph->getVertexBiNeighbors(data_vertex, d_neighbors_num);
                            for (unsigned ii = 0; ii < q_bi_neighbors_num; ++ii)
                            {
                                q_neighbor_vertex = q_bi_neighbors[ii];
                                q_motif_count = query_graph->getVertexMotifCount(q_neighbor_vertex);
                                unsigned jj;
                                for (jj = 0; jj < d_neighbors_num; ++jj)
                                {
                                    d_neighbor_vertex = d_neighbors[jj];
                                    d_motif_count = data_graph->getVertexMotifCount(d_neighbor_vertex);
                                    if (arr_cover(d_motif_count, q_motif_count, MOTIF_COUNT_DEMENSION))
                                    { //d_neighbor_vertex covers q_neighbor_vertex
                                        break;
                                    }
                                }
                                if (jj >= d_neighbors_num)
                                { //q_neighbor_vertex didn't find any cover in d_neighbors
                                    q_candidates[j] = INVALID_VERTEX_ID;
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    q_candidates[j] = INVALID_VERTEX_ID;
                    data_vertex = INVALID_VERTEX_ID;
                }
            }
        }
    }

    /*
    Compact candidates
    */
    compactCandidates(candidates, candidates_count, query_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, query_vertex_num);
}

#endif // TOPO_MOTIF_ENABLE == 1

#if LABEL_MOTIF_ENABLE == 1
bool Filter::NLFFilter(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count)
{
    allocateBuffer(data_graph, query_graph, candidates, candidates_count);

    for (unsigned i = 0; i < query_graph->getVerticesCount(); ++i)
    {
        VertexID query_vertex = i;
        computeCandidateWithNLF(data_graph, query_graph, query_vertex, candidates_count[query_vertex], candidates[query_vertex]);
        if (candidates_count[query_vertex] == 0)
        {
            return false;
        }
    }

    return true;
}

bool Filter::NLFFilter1step(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count)
{
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //NLF
        return false;
    unsigned q_vertex_num = query_graph->getVerticesCount();
    for (unsigned q_v = 0; q_v < q_vertex_num; ++q_v)
    {
        unsigned in_count, out_count, bi_count;
        const unsigned *q_in_neighbors = query_graph->getVertexInNeighbors(q_v, in_count);
        const unsigned *q_out_neighbors = query_graph->getVertexOutNeighbors(q_v, out_count);
        const unsigned *q_bi_neighbors = query_graph->getVertexBiNeighbors(q_v, bi_count);
        unsigned candi_num = candidates_count[q_v];
        for (unsigned j = 0; j < candi_num; ++j)
        {
            unsigned d_v = candidates[q_v][j];
            if (d_v != INVALID_VERTEX_ID)
            {
                unsigned d_in_count, d_out_count, d_bi_count;
                const unsigned *d_in_neighbors = data_graph->getVertexInNeighbors(d_v, d_in_count);
                const unsigned *d_out_neighbors = data_graph->getVertexOutNeighbors(d_v, d_out_count);
                const unsigned *d_bi_neighbors = data_graph->getVertexBiNeighbors(d_v, d_bi_count);
#if STEP_DEBUG == 1
                std::cout << "q_v:" << q_v << "\nd_v:" << d_v << std::endl;
#endif //STEP_DEBUG==1

                if (check_nlf_cover(in_count, d_in_count, q_in_neighbors, d_in_neighbors, query_graph, data_graph) && check_nlf_cover(out_count, d_out_count, q_out_neighbors, d_out_neighbors, query_graph, data_graph) && check_nlf_cover(bi_count, d_bi_count, q_bi_neighbors, d_bi_neighbors, query_graph, data_graph))
                {
                }
                else
                {
                    candidates[q_v][j] = INVALID_VERTEX_ID;
                }
            }
        }
    }
    compactCandidates(candidates, candidates_count, q_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, q_vertex_num);
}

/* WARNNING:此函数实现的逻辑为数据图固定存k维，已经注释掉，即此函数尚未实现
1.vertex label considered
2.data_graph,query_graph: motif structure already loaded
3.data_graph: k already loaded
4.will allocate space for candidates and candidates_count
*/
bool Filter::LabelMotifFilter(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count)
{
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
        return false;

    unsigned query_vertex_num = query_graph->getVerticesCount();
    unsigned data_vertex_num = data_graph->getVerticesCount();

    /*
    motif filter
    */
    unsigned impossible_code = (1 << (2 * LABEL_BIT_WIDTH));

    VertexID *q_candidates;
    unsigned q_candidates_num;

    //label_motif_count_
    /* 注释部分实现逻辑为数据图固定存k维
    Lmtf *q_lmtf_arr, *g_lmtf_arr;
    unsigned q_lmtf_arr_sz;
    unsigned k = data_graph->getK();
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        q_lmtf_arr = query_graph->getVertexLabelMotifCount(i, q_lmtf_arr_sz);
        for (unsigned j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                g_lmtf_arr = data_graph->getVertexLabelMotifCount(data_vertex);
                if (lmtf_arr_cover(g_lmtf_arr, k, q_lmtf_arr, q_lmtf_arr_sz) == 0)
                {
                    q_candidates[j] = INVALID_VERTEX_ID;
                }
            }
        }
    }
    */

    /*
    Compact candidates
    */
    compactCandidates(candidates, candidates_count, query_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, query_vertex_num);
}

//思路：归并
bool Filter::lmtf_arr_cover(const Lmtf *g_lmtf_arr, unsigned k, const Lmtf *q_lmtf_arr, unsigned q_lmtf_arr_sz)
{
    //q_lmtf_arr g_lmtf_arr: code ascending
    unsigned impossible_code = (1 << (2 * LABEL_BIT_WIDTH));
    unsigned b = 0; //skip impossible_code slice in g_lmtf_arr head
    while (b < k && g_lmtf_arr[b].code == impossible_code)
    {
        ++b;
    }
    unsigned idxg = b;
    for (unsigned ii = 0; idxg < k && ii < q_lmtf_arr_sz; ++ii)
    {
        unsigned qcode = q_lmtf_arr[ii].code;
        unsigned qcnt = q_lmtf_arr[ii].cnt;
        unsigned tmpcode;
        while (idxg < k)
        {
            tmpcode = g_lmtf_arr[idxg].code;
            if (tmpcode == impossible_code)
            {
                break;
            }
            if (tmpcode == qcode)
            {
                if (qcnt > g_lmtf_arr[idxg].cnt)
                {
                    return 0;
                }
                break;
            }
            if (tmpcode > qcode)
            {
                break;
            }
            ++idxg; //tmpcode<qcode
        }
        if (tmpcode == impossible_code)
        {
            break; //skip impossible_code slice in g_lmtf_arr tail
        }
    }
    return 1;
    /*
                unsigned b = 0, e = q_lmtf_arr_sz - 1, idx;
                int new_e;

                unsigned ii = 0;
                //skip impossible code
                while (g_lmtf_arr[ii].code == impossible_code)
                {
                    ++ii;
                }
                //g_lmtf_arr[ii].code != impossible_code
                for (; ii < k; ++ii) //data_vertex in G记录的每个特征
                {
                    unsigned code = g_lmtf_arr[ii].code;
                    if (code == impossible_code)
                    {
                        break; //尾段impossible code,skip
                    }
                    idx = bi_search(q_lmtf_arr, b, e, code, new_e); //[b,e]search
                    if (idx > e || idx < b || new_e == -1)
                    { //大于所有或小于所有类型的找不到
                        break;
                    }
                    if (code == q_lmtf_arr[idx].code)
                    {
                        if (g_lmtf_arr[ii].cnt < q_lmtf_arr[idx].cnt)
                        {
                            q_candidates[ii] = INVALID_VERTEX_ID;
                        }
                        ++idx;
                    }
                    if (new_e >= 0)
                    {
                        e = new_e;
                    }
                    b = idx;
                }
                */
}

/* WARNNING:此函数实现的依赖的LabelMotifFilter函数的逻辑为数据图固定存k维，已经注释掉，即此函数尚未实现
*/
bool Filter::LabelMotifFilter1step(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count)
{
    if (!LabelMotifFilter(data_graph, query_graph, candidates, candidates_count)) //NLF
        return false;
    unsigned q_vertex_num = query_graph->getVerticesCount();
    for (unsigned q_v = 0; q_v < q_vertex_num; ++q_v)
    {
        unsigned in_count, out_count, bi_count;
        const unsigned *q_in_neighbors = query_graph->getVertexInNeighbors(q_v, in_count);
        const unsigned *q_out_neighbors = query_graph->getVertexOutNeighbors(q_v, out_count);
        const unsigned *q_bi_neighbors = query_graph->getVertexBiNeighbors(q_v, bi_count);
        unsigned candi_num = candidates_count[q_v];
        for (unsigned j = 0; j < candi_num; ++j)
        {
            unsigned d_v = candidates[q_v][j];
            if (d_v != INVALID_VERTEX_ID)
            {
                unsigned d_in_count, d_out_count, d_bi_count;
                const unsigned *d_in_neighbors = data_graph->getVertexInNeighbors(d_v, d_in_count);
                const unsigned *d_out_neighbors = data_graph->getVertexOutNeighbors(d_v, d_out_count);
                const unsigned *d_bi_neighbors = data_graph->getVertexBiNeighbors(d_v, d_bi_count);
                if (check_lmtf_cover(in_count, d_in_count, q_in_neighbors, d_in_neighbors, query_graph, data_graph) && check_lmtf_cover(out_count, d_out_count, q_out_neighbors, d_out_neighbors, query_graph, data_graph) && check_lmtf_cover(bi_count, d_bi_count, q_bi_neighbors, d_bi_neighbors, query_graph, data_graph))
                {
                }
                else
                {
                    candidates[q_v][j] = INVALID_VERTEX_ID;
                }
            }
        }
    }
    compactCandidates(candidates, candidates_count, q_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, q_vertex_num);
}

/* WARNNING:此函数实现的依赖的逻辑为数据图固定存k维，已经注释掉，即此函数尚未实现
*/
bool Filter::check_lmtf_cover(unsigned in_count, unsigned d_in_count, const unsigned *q_in_neighbors, const unsigned *d_in_neighbors, const MotifQ *query_graph, const MotifG *data_graph)
{
    for (unsigned ii = 0; ii < in_count; ++ii) //for all q_v_n, find covers in d_v_n
    {
        unsigned q_v_n = q_in_neighbors[ii];
        const std::unordered_map<LabelID, unsigned> *q_in_map = query_graph->getVertexInNLF(q_v_n);
        const std::unordered_map<LabelID, unsigned> *q_out_map = query_graph->getVertexOutNLF(q_v_n);
        const std::unordered_map<LabelID, unsigned> *q_bi_map = query_graph->getVertexBiNLF(q_v_n);
        unsigned q_in_d = query_graph->getVertexInDegree(q_v_n);
        unsigned q_out_d = query_graph->getVertexOutDegree(q_v_n);
        unsigned q_bi_d = query_graph->getVertexBiDegree(q_v_n);
        LabelID q_v_n_label = query_graph->getVertexLabel(q_v_n);
        unsigned k; // = data_graph->getK();

        unsigned q_lmtf_arr_sz;
        const Lmtf *q_lmtf_arr = query_graph->getVertexLabelMotifCount(q_v_n, q_lmtf_arr_sz);

        unsigned jj;
        for (jj = 0; jj < d_in_count; ++jj)
        {
            unsigned d_v_n = d_in_neighbors[jj];
            if (data_graph->getVertexLabel(d_v_n) == q_v_n_label)
            {
#if STEP_DEBUG == 1
                std::cout << "q_v_n:" << q_v_n << "\nd_v_n:" << d_v_n << std::endl;
#endif //#if STEP_DEBUG == 1
                if (((data_graph->getVertexInDegree(d_v_n)) >= q_in_d) && ((data_graph->getVertexOutDegree(d_v_n)) >= q_out_d) && ((data_graph->getVertexBiDegree(d_v_n)) >= q_bi_d))
                {

                    const std::unordered_map<LabelID, unsigned> *d_in_map = data_graph->getVertexInNLF(d_v_n);
                    const std::unordered_map<LabelID, unsigned> *d_out_map = data_graph->getVertexOutNLF(d_v_n);
                    const std::unordered_map<LabelID, unsigned> *d_bi_map = data_graph->getVertexBiNLF(d_v_n);
                    const Lmtf *g_lmtf_arr = data_graph->getVertexLabelMotifCount(d_v_n);
                    if (map_cover(d_in_map, q_in_map) && map_cover(d_out_map, q_out_map) && map_cover(d_bi_map, q_bi_map) && lmtf_arr_cover(g_lmtf_arr, k, q_lmtf_arr, q_lmtf_arr_sz))
                    {
                        break; //q_v_n find a cover in d_v_n}
                    }
                }
            }
        }
        if (jj >= d_in_count)
        {
            return 0;
        }
    }
    return 1;
}

/* WARNNING:此函数实现的依赖的逻辑为数据图固定存k维，已经注释掉，即此函数尚未实现
[q_v特征中在G中找到的数目,q_v的总特征数,
记录为可以覆盖的q_v当中覆盖的q_v的特征,记录为可以覆盖的q_v当中q_v的总特征数]
为统计时间服务
*/
bool Filter::LabelMotifFilter_collect_data_feature(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, unsigned *feature)
{
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
        return false;

    unsigned query_vertex_num = query_graph->getVerticesCount();
    unsigned data_vertex_num = data_graph->getVerticesCount();

    /*
    motif filter
    */
    unsigned impossible_code = (1 << (2 * LABEL_BIT_WIDTH));

    VertexID *q_candidates;
    unsigned q_candidates_num;
    //label_motif_count_
    Lmtf *q_lmtf_arr, *g_lmtf_arr;
    unsigned q_lmtf_arr_sz;
    unsigned k; // = data_graph->getK();
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        q_lmtf_arr = query_graph->getVertexLabelMotifCount(i, q_lmtf_arr_sz);
        for (unsigned j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                g_lmtf_arr = data_graph->getVertexLabelMotifCount(data_vertex);
                if (lmtf_arr_cover_collect_data_feature(g_lmtf_arr, k, q_lmtf_arr, q_lmtf_arr_sz, feature) == 0)
                {
                    q_candidates[j] = INVALID_VERTEX_ID;
                }
            }
        }
    }

    /*
    Compact candidates
    */
    compactCandidates(candidates, candidates_count, query_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, query_vertex_num);
}

/*
[q_v特征中在G中找到的且cnt!=1数目,q_v的总特征数,
记录为可以覆盖的q_v当中q_v特征中在G中找到的且cnt!=1数目,记录为可以覆盖的q_v当中q_v的总特征数]
*/
bool Filter::lmtf_arr_cover_collect_data_feature(const Lmtf *g_lmtf_arr, unsigned k, const Lmtf *q_lmtf_arr, unsigned q_lmtf_arr_sz, unsigned *feature)
{
    //q_lmtf_arr g_lmtf_arr: code ascending
    unsigned impossible_code = (1 << (2 * LABEL_BIT_WIDTH));
    unsigned b = 0; //skip impossible_code slice in g_lmtf_arr head
    while (b < k && g_lmtf_arr[b].code == impossible_code)
    {
        ++b;
    }
    unsigned idxg = b;

    unsigned cover_feature_cnt = 0; //记录为可以覆盖的q_v当中覆盖的q_v的特征
    feature[1] += q_lmtf_arr_sz;    //q_v的总特征数

    bool flag = 1;

    for (unsigned ii = 0; idxg < k && ii < q_lmtf_arr_sz; ++ii)
    {
        unsigned qcode = q_lmtf_arr[ii].code;
        unsigned qcnt = q_lmtf_arr[ii].cnt;
        unsigned tmpcode;
        while (idxg < k)
        {
            tmpcode = g_lmtf_arr[idxg].code;
            if (tmpcode == impossible_code)
            {
                break;
            }
            if (tmpcode == qcode)
            {
                if (qcnt > 1)
                {
                    cover_feature_cnt++; //q_v特征中cnt!=1的在G中找到的数目
                }
                if (qcnt > g_lmtf_arr[idxg].cnt)
                {
                    flag = 0;
                    break;
                }
                break;
            }
            if (tmpcode > qcode)
            {
                break;
            }
            ++idxg; //tmpcode<qcode
        }
        if (tmpcode == impossible_code)
        {
            break; //skip impossible_code slice in g_lmtf_arr tail
        }
    }
    feature[0] += cover_feature_cnt; //q_v特征中在G中找到的数目
    if (flag)
    {
        //q_v被记录为被覆盖
        feature[2] += cover_feature_cnt;
        feature[3] += q_lmtf_arr_sz;
    }
    return flag;
}

#if LABEL_MOTIF_LIMIT == 1
bool Filter::LabelMotifFilter_limit(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count)
{
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
        return false;

    unsigned query_vertex_num = query_graph->getVerticesCount();
    unsigned data_vertex_num = data_graph->getVerticesCount();

    /*
    motif filter
    */
    VertexID *q_candidates;
    unsigned q_candidates_num;

    //label_motif_map_
    std::map<unsigned, unsigned> *q_lmtf_map, *g_lmtf_map;
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        q_lmtf_map = query_graph->getVertexLabelMotifMap(i);
        for (unsigned j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                g_lmtf_map = data_graph->getVertexLabelMotifMap(data_vertex);
                for (auto item : *q_lmtf_map)
                {
                    unsigned code = item.first;
                    if (g_lmtf_map->count(code) == 0 || (*g_lmtf_map)[code] < item.second)
                    {
                        q_candidates[j] = INVALID_VERTEX_ID;
                    }
                }
            }
        }
    }

    /*
    Compact candidates
    */
    compactCandidates(candidates, candidates_count, query_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, query_vertex_num);
}
#endif //LABEL_MOTIF_LIMIT==1
#endif // LABEL_MOTIF_ENABLE == 1

double Filter::LDFFilter_AveCandiScale(const MotifG *data_graph, const MotifQ *query_graph)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();
    if (Filter::LDFFilter(data_graph, query_graph, candidates, candidates_count))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    print_candidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}

#if TOPO_MOTIF_ENABLE == 1
double Filter::TopoMotifFilter_AveCandiScale(const MotifG *data_graph, MotifQ *query_graph, std::string filename)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();

    //给查询图计算需要的结构
    query_graph->BuildNLF();
    bool genTMTFsuccess;
#if WRITE_TO_FILE_DEBUG == 1
    if (filename == "")
    {
        std::cout << __FILE__ << " :" << __LINE__;
        std::cout << "\n\tin TopoMotifFilter_AveCandiScale: no filename specified, return\n";
        return -1;
    }
    genTMTFsuccess = query_graph->generateTopoMotifCount(filename.substr(0, filename.find_last_of('.')));
#else
    genTMTFsuccess = query_graph->generateTopoMotifCount();
#endif //WRITE_TO_FILE_DEBUG==1
    if (genTMTFsuccess == 0)
    {
        std::cout << __FILE__ << " :" << __LINE__;
        std::cout << "\n\tin TopoMotifFilter_AveCandiScale: generateTopoMotifCount for query_graph failed, return\n";
        return -1;
    }

    /* WARNNING: 传参数1硬编码使用NLF*/
    if (Filter::TopoMotifFilter(data_graph, query_graph, candidates, candidates_count, 1))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    print_candidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}
double Filter::TopoMotifFilter1step_AveCandiScale(const MotifG *data_graph, MotifQ *query_graph, std::string filename)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();

    //给查询图计算需要的结构
    query_graph->BuildNLF();
    bool genTMTFsuccess;
#if WRITE_TO_FILE_DEBUG == 1
    if (filename == "")
    {
        std::cout << __FILE__ << " :" << __LINE__;
        std::cout << "\n\tin TopoMotifFilter_AveCandiScale: no filename specified, return\n";
        return -1;
    }
    genTMTFsuccess = query_graph->generateTopoMotifCount(filename.substr(0, filename.find_last_of('.')));
#else
    genTMTFsuccess = query_graph->generateTopoMotifCount();
#endif //WRITE_TO_FILE_DEBUG==1
    if (genTMTFsuccess == 0)
    {
        std::cout << __FILE__ << " :" << __LINE__;
        std::cout << "\n\tin TopoMotifFilter_AveCandiScale: generateTopoMotifCount for query_graph failed, return\n";
        return -1;
    }

    /* WARNNING: 传参数1硬编码使用NLF*/
    if (Filter::TopoMotifFilter1step(data_graph, query_graph, candidates, candidates_count, 1))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    print_candidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}

#endif //#if TOPO_MOTIF_ENABLE == 1
/* WARNNING: 没有实现普通labelMotifFilter的这个函数
*/
#if LABEL_MOTIF_ENABLE == 1
double Filter::NLFFilter_AveCandiScale(const MotifG *data_graph, MotifQ *query_graph)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();
    //给查询图计算需要的结构
    query_graph->BuildNLF();
    if (Filter::NLFFilter(data_graph, query_graph, candidates, candidates_count))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    print_candidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}
double Filter::NLFFilter1step_AveCandiScale(const MotifG *data_graph, MotifQ *query_graph)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();
    //给查询图计算需要的结构
    query_graph->BuildNLF();
    if (Filter::NLFFilter1step(data_graph, query_graph, candidates, candidates_count))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    print_candidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}
#if LABEL_MOTIF_LIMIT == 1
double Filter::LabelMotifFilter_limit_AveCandiScale(const MotifG *data_graph, MotifQ *query_graph, std::string filename)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();

    //给查询图计算需要的结构
    query_graph->BuildNLF();
    bool genMTFsuccess;
#if WRITE_TO_FILE_DEBUG == 1
    if (filename == "")
    {
        std::cout << __FILE__ << " :" << __LINE__;
        std::cout << "\n\tin LabelMotifFilter_limit_AveCandiScale: no filename specified, return\n";
        return -1;
    }
    genMTFsuccess = query_graph->generateLabelMotifCount_limit(filename.substr(0, filename.find_last_of('.')));
#else
    genMTFsuccess = query_graph->generateLabelMotifCount_limit();
#endif //WRITE_TO_FILE_DEBUG==1
    if (genMTFsuccess == 0)
    {
        std::cout << __FILE__ << " :" << __LINE__;
        std::cout << "\n\tin LabelMotifFilter_limit_AveCandiScale: generateLabelMotifCount_limit for query_graph failed, return\n";
        return -1;
    }

    if (Filter::LabelMotifFilter_limit(data_graph, query_graph, candidates, candidates_count))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    print_candidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}
#endif //#if LABEL_MOTIF_LIMIT == 1
#endif //#if LABEL_MOTIF_ENABLE == 1

void Filter::computeCandidateWithNLF(const MotifG *data_graph, const MotifQ *query_graph, VertexID query_vertex,
                                     unsigned &count, unsigned *buffer) //count buffer: candidates_count[query_vertex], candidates[query_vertex]
{
    LabelID label = query_graph->getVertexLabel(query_vertex);
    unsigned indegree = query_graph->getVertexInDegree(query_vertex);
    unsigned outdegree = query_graph->getVertexOutDegree(query_vertex);
    unsigned bidegree = query_graph->getVertexBiDegree(query_vertex);

    const std::unordered_map<LabelID, unsigned> *query_vertex_in_nlf = query_graph->getVertexInNLF(query_vertex);
    const std::unordered_map<LabelID, unsigned> *query_vertex_out_nlf = query_graph->getVertexOutNLF(query_vertex);
    const std::unordered_map<LabelID, unsigned> *query_vertex_bi_nlf = query_graph->getVertexBiNLF(query_vertex);

    unsigned data_vertex_num;
    const unsigned *data_vertices = data_graph->getVerticesByLabel(label, data_vertex_num);
    count = 0;
    for (unsigned j = 0; j < data_vertex_num; ++j)
    {
        unsigned data_vertex = data_vertices[j];
        if ((data_graph->getVertexInDegree(data_vertex) >= indegree) && (data_graph->getVertexOutDegree(data_vertex) >= outdegree) && (data_graph->getVertexBiDegree(data_vertex) >= bidegree))
        {
            // NLF check
            const std::unordered_map<LabelID, unsigned> *data_vertex_in_nlf = data_graph->getVertexInNLF(data_vertex);
            const std::unordered_map<LabelID, unsigned> *data_vertex_out_nlf = data_graph->getVertexOutNLF(data_vertex);
            const std::unordered_map<LabelID, unsigned> *data_vertex_bi_nlf = data_graph->getVertexBiNLF(data_vertex);
            if ((data_vertex_in_nlf->size() >= query_vertex_in_nlf->size()) && (data_vertex_out_nlf->size() >= query_vertex_out_nlf->size()) && (data_vertex_bi_nlf->size() >= query_vertex_bi_nlf->size()))
            {
                if (map_cover(data_vertex_in_nlf, query_vertex_in_nlf) && map_cover(data_vertex_out_nlf, query_vertex_out_nlf) && map_cover(data_vertex_bi_nlf, query_vertex_bi_nlf)) //succeed the NLF test, add to candi
                {
                    if (buffer != NULL)
                    {
                        buffer[count] = data_vertex;
                    }
                    count += 1;
                }
            }
        }
    }
}

void Filter::compactCandidates(unsigned **&candidates, unsigned *&candidates_count, unsigned query_vertex_num)
{
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        VertexID query_vertex = i;
        unsigned next_position = 0;
        for (unsigned j = 0; j < candidates_count[query_vertex]; ++j)
        {
            VertexID data_vertex = candidates[query_vertex][j];

            if (data_vertex != INVALID_VERTEX_ID)
            {
                candidates[query_vertex][next_position++] = data_vertex;
            }
        }

        candidates_count[query_vertex] = next_position;
    }
}

bool Filter::isCandidateSetValid(unsigned **&candidates, unsigned *&candidates_count, unsigned query_vertex_num)
{
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        if (candidates_count[i] == 0)
            return false;
    }
    return true;
}

void Filter::allocateBuffer(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates,
                            unsigned *&candidates_count)
{
    unsigned query_vertex_num = query_graph->getVerticesCount();
    unsigned candidates_max_num = data_graph->getGraphMaxLabelFrequency();

    candidates_count = new unsigned[query_vertex_num];
    memset(candidates_count, 0, sizeof(unsigned) * query_vertex_num);

    candidates = new unsigned *[query_vertex_num];

    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        candidates[i] = new unsigned[candidates_max_num];
    }
}
/*
q_lmtf_arr[b,e] 二分找code为code的，返回下标
如果找不到，返回大于其的最近
q_lmtf_arr已经按code升序，后面可能会出现长串的impossible_code
如果判断是impossible_code而调整e，则会int引用返回e
*/

unsigned Filter::bi_search(Lmtf *q_lmtf_arr, unsigned ub, unsigned ue, unsigned code, int &new_e)
{
    unsigned impossible_code = (1 << (2 * LABEL_BIT_WIDTH));
    unsigned tmpcode;
    int b = (int(ub)) - 1;
    int e = (int(ue)) + 1;
    new_e = -2;
    while (b + 1 < e)
    {
        int mid = (b + e) / 2;
        tmpcode = q_lmtf_arr[mid].code;
        if (tmpcode == code)
        {
            return mid;
        }
        if (tmpcode == impossible_code)
        {
            e = mid;
            new_e = mid - 1;
        }
        else
        {
            if (code < tmpcode)
            {
                e = mid;
            }
            else
            {
                b = mid;
            }
        }
    }
    return e;
}

/*
candidates_count:vertex_num*1  candidates:vertex_num*[candidates_count]
*/
void Filter::print_candi(unsigned **candidates, unsigned *candidates_count, unsigned vertex_num)
{
    std::ofstream fout("candidates.txt");
    for (unsigned i = 0; i < vertex_num; ++i)
    {
        fout << i + 1;
        for (unsigned j = 0; j < candidates_count[i]; ++j)
        {
            fout << "\t" << candidates[i][j] + 1;
        }
        fout << "\n";
    }
    fout.close();
}

bool Filter::arr_cover(const unsigned *darr, const unsigned *qarr, unsigned sz)
{
    for (unsigned i = 0; i < sz; ++i)
    {
        if (darr[i] < qarr[i])
        {
            return 0;
        }
    }
    return 1;
}

//dmap covers qmap
bool Filter::map_cover(const std::unordered_map<LabelID, unsigned> *dmap, const std::unordered_map<LabelID, unsigned> *qmap)
{
#if STEP_DEBUG == 1
    std::cout << "dmap:\nsz:" << dmap->size() << std::endl;
    for (auto it : *dmap)
    {
        std::cout << "(" << it.first << ":" << it.second << ")\n";
    }
    std::cout << "\nqmap:\nsz:" << qmap->size() << "\n";
    for (auto it : *qmap)
    {
        std::cout << "(" << it.first << ":" << it.second << ")\n";
    }
    std::cout << std::endl;
#endif //#if STEP_DEBUG == 1
    if (dmap->size() < qmap->size())
    {
        return 0;
    }
    //loop qmap, find key in dmap
    for (auto qit : *qmap)
    {
        auto it = dmap->find(qit.first);
        if (it == dmap->end() || (it->second) < qit.second)
        {
            return 0;
        }
    }
    return 1;
}

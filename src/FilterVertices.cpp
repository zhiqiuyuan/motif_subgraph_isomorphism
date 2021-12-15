#include "../inc/FilterVertices.h"
bool FilterVertices::LDFFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count)
{
    allocateBuffer(data_graph, query_graph, candidates, candidates_count);

    for (ui i = 0; i < query_graph->getVerticesCount(); ++i)
    {
        LabelID label = query_graph->getVertexLabel(i);

        ui data_vertex_num;
        const ui *data_vertices = data_graph->getVerticesByLabel(label, data_vertex_num);
#if DIRECTED_GRAPH == 0
        ui degree = query_graph->getVertexDegree(i);
        for (ui j = 0; j < data_vertex_num; ++j)
        {
            ui data_vertex = data_vertices[j];
            if (data_graph->getVertexDegree(data_vertex) >= degree)
            {
                candidates[i][candidates_count[i]++] = data_vertex;
            }
        }
#else
        ui indegree = query_graph->getVertexInDegree(i);
        ui outdegree = query_graph->getVertexOutDegree(i);
        ui bidegree = query_graph->getVertexBiDegree(i);
        for (ui j = 0; j < data_vertex_num; ++j)
        {
            ui data_vertex = data_vertices[j];
            if ((data_graph->getVertexInDegree(data_vertex) >= indegree) && (data_graph->getVertexOutDegree(data_vertex) >= outdegree) && (data_graph->getVertexBiDegree(data_vertex) >= bidegree))
            {
                candidates[i][candidates_count[i]++] = data_vertex;
            }
        }
#endif // DIRECTED_GRAPH

        if (candidates_count[i] == 0)
        {
            return false;
        }
    }

    return true;
}

bool FilterVertices::NLFFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count)
{
    allocateBuffer(data_graph, query_graph, candidates, candidates_count);

    for (ui i = 0; i < query_graph->getVerticesCount(); ++i)
    {
        VertexID query_vertex = i;
#if DIRECTED_GRAPH == 1
        computeCandidateWithNLF_directed(data_graph, query_graph, query_vertex, candidates_count[query_vertex], candidates[query_vertex]);
#else
        computeCandidateWithNLF(data_graph, query_graph, query_vertex, candidates_count[query_vertex], candidates[query_vertex]);
#endif
        if (candidates_count[query_vertex] == 0)
        {
            return false;
        }
    }

    return true;
}

bool FilterVertices::NLFFilter_1step(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count)
{
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //NLF
        return false;
    ui q_vertex_num = query_graph->getVerticesCount();
    for (ui q_v = 0; q_v < q_vertex_num; ++q_v)
    {
        ui in_count, out_count, bi_count;
        const ui *q_in_neighbors = query_graph->getVertexInNeighbors(q_v, in_count);
        const ui *q_out_neighbors = query_graph->getVertexOutNeighbors(q_v, out_count);
        const ui *q_bi_neighbors = query_graph->getVertexBiNeighbors(q_v, bi_count);
        ui candi_num = candidates_count[q_v];
        for (ui j = 0; j < candi_num; ++j)
        {
            ui d_v = candidates[q_v][j];
            if (d_v != INVALID_VERTEX_ID)
            {
                ui d_in_count, d_out_count, d_bi_count;
                const ui *d_in_neighbors = data_graph->getVertexInNeighbors(d_v, d_in_count);
                const ui *d_out_neighbors = data_graph->getVertexOutNeighbors(d_v, d_out_count);
                const ui *d_bi_neighbors = data_graph->getVertexBiNeighbors(d_v, d_bi_count);
#ifdef ONLINE_DEBUG
                std::cout << "q_v:" << q_v << "\nd_v:" << d_v << std::endl;
#endif //ONLINE_DEBUG

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

bool FilterVertices::check_nlf_cover(ui in_count, ui d_in_count, const ui *q_in_neighbors, const ui *d_in_neighbors, const Graph *query_graph, const Graph *data_graph)
{
    for (ui ii = 0; ii < in_count; ++ii) //for all q_v_n, find covers in d_v_n
    {
        ui q_v_n = q_in_neighbors[ii];
        const std::unordered_map<LabelID, ui> *q_in_map = query_graph->getVertexInNLF(q_v_n);
        const std::unordered_map<LabelID, ui> *q_out_map = query_graph->getVertexOutNLF(q_v_n);
        const std::unordered_map<LabelID, ui> *q_bi_map = query_graph->getVertexBiNLF(q_v_n);
        ui q_in_d = query_graph->getVertexInDegree(q_v_n);
        ui q_out_d = query_graph->getVertexOutDegree(q_v_n);
        ui q_bi_d = query_graph->getVertexBiDegree(q_v_n);
        LabelID q_v_n_label = query_graph->getVertexLabel(q_v_n);

        ui jj;
        for (jj = 0; jj < d_in_count; ++jj)
        {
            ui d_v_n = d_in_neighbors[jj];
            if (data_graph->getVertexLabel(d_v_n) == q_v_n_label)
            {
#ifdef ONLINE_DEBUG
                std::cout << "q_v_n:" << q_v_n << "\nd_v_n:" << d_v_n << std::endl;
#endif //ONLINE_DEBUG
                if (((data_graph->getVertexInDegree(d_v_n)) >= q_in_d) && ((data_graph->getVertexOutDegree(d_v_n)) >= q_out_d) && ((data_graph->getVertexBiDegree(d_v_n)) >= q_bi_d))
                {

                    const std::unordered_map<LabelID, ui> *d_in_map = data_graph->getVertexInNLF(d_v_n);
                    const std::unordered_map<LabelID, ui> *d_out_map = data_graph->getVertexOutNLF(d_v_n);
                    const std::unordered_map<LabelID, ui> *d_bi_map = data_graph->getVertexBiNLF(d_v_n);
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
bool FilterVertices::MotifFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count, bool use_nlf)
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

    //print_candi(candidates, candidates_count, query_graph->getVerticesCount());

    ui query_vertex_num = query_graph->getVerticesCount();
    ui data_vertex_num = data_graph->getVerticesCount();

    /*
    motif filter
    */
    VertexID *q_candidates;
    ui q_candidates_num;
#if DIRECTED_GRAPH == 0 //undirected
    //tri_count_
    ui q_tri_cnt;
    for (ui i = 0; i < query_vertex_num; ++i)
    {
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        q_tri_cnt = query_graph->getVertexTriCount(i);
        for (ui j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                if (data_graph->getVertexTriCount(data_vertex) < q_tri_cnt)
                {
                    q_candidates[j] = INVALID_VERTEX_ID;
                }
            }
        }
    }
#else  //directed
    //motif_count_
    ui *q_motif_count, *g_motif_count;
    for (ui i = 0; i < query_vertex_num; ++i)
    {
        //VertexID query_vertex = i;
        q_motif_count = query_graph->getVertexMotifCount(i);
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        for (ui j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                g_motif_count = data_graph->getVertexMotifCount(data_vertex);
                for (ui k = 0; k < MOTIF_COUNT_DEMENSION; ++k)
                {
                    if (g_motif_count[k] < q_motif_count[k])
                    {
                        q_candidates[j] = INVALID_VERTEX_ID;
                    }
                }
            }
        }
    }
#endif //if DIRECTED_GRAPH==0

    /*
    Compact candidates
    */
    compactCandidates(candidates, candidates_count, query_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, query_vertex_num);
}
/*
1.MotifFilter first, then one step filtering according to topo motif count
*/
bool FilterVertices::MotifFilter_1step(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count, bool use_nlf)
{
    if (!MotifFilter(data_graph, query_graph, candidates, candidates_count, use_nlf))
    {
        //ldf + topo motif filter only on this, without considering neighbors
        return false;
    }

    ui query_vertex_num = query_graph->getVerticesCount();
    ui data_vertex_num = data_graph->getVerticesCount();

    /*
    one step motif filter
    */
    VertexID *q_candidates;
    ui q_candidates_num;
#if DIRECTED_GRAPH == 0
#else  //directed
    //motif_count_
    ui *q_motif_count, *d_motif_count;
    const ui *q_in_neighbors, *q_out_neighbors, *q_bi_neighbors;
    ui q_in_neighbors_num, q_out_neighbors_num, q_bi_neighbors_num;
    ui q_neighbor_vertex, d_neighbor_vertex;
    for (ui i = 0; i < query_vertex_num; ++i)
    {
        VertexID query_vertex = i;
        q_candidates = candidates[query_vertex];
        q_candidates_num = candidates_count[query_vertex];

        q_in_neighbors = query_graph->getVertexInNeighbors(query_vertex, q_in_neighbors_num);
        q_out_neighbors = query_graph->getVertexOutNeighbors(query_vertex, q_out_neighbors_num);
        q_bi_neighbors = query_graph->getVertexBiNeighbors(query_vertex, q_bi_neighbors_num);

        for (ui j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                ui d_in_count, d_out_count, d_bi_count;
                const ui *d_in_neighbors = data_graph->getVertexInNeighbors(data_vertex, d_in_count);
                const ui *d_out_neighbors = data_graph->getVertexOutNeighbors(data_vertex, d_out_count);
                const ui *d_bi_neighbors = data_graph->getVertexBiNeighbors(data_vertex, d_bi_count);
                if (check_nlf_cover(q_in_neighbors_num, d_in_count, q_in_neighbors, d_in_neighbors, query_graph, data_graph) && check_nlf_cover(q_out_neighbors_num, d_out_count, q_out_neighbors, d_out_neighbors, query_graph, data_graph) && check_nlf_cover(q_bi_neighbors_num, d_bi_count, q_bi_neighbors, d_bi_neighbors, query_graph, data_graph))
                {
                    ui d_neighbors_num;
                    //in
                    const ui *d_neighbors = data_graph->getVertexInNeighbors(data_vertex, d_neighbors_num);
                    //in_neighbors of data_vertex need to cover every in_neighbor of query_vertex
                    for (ui ii = 0; ii < q_in_neighbors_num; ++ii)
                    {
                        q_neighbor_vertex = q_in_neighbors[ii];
                        q_motif_count = query_graph->getVertexMotifCount(q_neighbor_vertex);
                        ui jj;
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
                        for (ui ii = 0; ii < q_out_neighbors_num; ++ii)
                        {
                            q_neighbor_vertex = q_out_neighbors[ii];
                            q_motif_count = query_graph->getVertexMotifCount(q_neighbor_vertex);
                            ui jj;
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
                            for (ui ii = 0; ii < q_bi_neighbors_num; ++ii)
                            {
                                q_neighbor_vertex = q_bi_neighbors[ii];
                                q_motif_count = query_graph->getVertexMotifCount(q_neighbor_vertex);
                                ui jj;
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
#endif //if DIRECTED_GRAPH==0

    /*
    Compact candidates
    */
    compactCandidates(candidates, candidates_count, query_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, query_vertex_num);
}

#endif // TOPO_MOTIF_ENABLE == 1

#if LABEL_MOTIF_ENABLE == 1
/*
1.vertex label considered
2.data_graph,query_graph: motif structure already loaded
3.data_graph: k already loaded
4.will allocate space for candidates and candidates_count
*/
bool FilterVertices::LabelMotifFilter(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count)
{
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
        return false;

    ui query_vertex_num = query_graph->getVerticesCount();
    ui data_vertex_num = data_graph->getVerticesCount();

    /*
    motif filter
    */
    ui impossible_code = (1 << (2 * LABEL_BIT_WIDTH));

    VertexID *q_candidates;
    ui q_candidates_num;
#if DIRECTED_GRAPH == 0 //undirected
    //label_tri_count_
    ui q_tri_cnt;
    std::map<ui, std::map<ui, ui>> *q_tri_cnt_map, *g_tri_cnt_map;
    for (ui i = 0; i < query_vertex_num; ++i)
    {
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        q_tri_cnt_map = query_graph->getVertexLabelTriCount(i);
        for (ui j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                g_tri_cnt_map = data_graph->getVertexLabelTriCount(data_vertex);
                for (auto item : *q_tri_cnt_map)
                {
                    for (auto it : item.second)
                    {
                        ui v1 = item.first;
                        ui v2 = it.first;
                        if (v1 > v2)
                        {
                            ui tmp = v1;
                            v1 = v2;
                            v2 = tmp;
                        }
                        if ((*g_tri_cnt_map).count(v1) == 0 || (*g_tri_cnt_map)[v1].count(v2) == 0 || (*g_tri_cnt_map)[v1][v2] < it.second)
                        {
                            q_candidates[j] = INVALID_VERTEX_ID;
                        }
                    }
                }
            }
        }
    }
#else  //directed \
       //label_motif_count_
    Lmtf *q_lmtf_arr, *g_lmtf_arr;
    ui q_lmtf_arr_sz;
    ui k = data_graph->getK();
    for (ui i = 0; i < query_vertex_num; ++i)
    {
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        q_lmtf_arr = query_graph->getVertexLabelMotifCount(i, q_lmtf_arr_sz);
        for (ui j = 0; j < q_candidates_num; ++j)
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
#endif // DIRECTED_GRAPH

    /*
    Compact candidates
    */
    compactCandidates(candidates, candidates_count, query_vertex_num);

    return isCandidateSetValid(candidates, candidates_count, query_vertex_num);
}

//思路：归并
bool FilterVertices::lmtf_arr_cover(const Lmtf *g_lmtf_arr, ui k, const Lmtf *q_lmtf_arr, ui q_lmtf_arr_sz)
{
    //q_lmtf_arr g_lmtf_arr: code ascending
    ui impossible_code = (1 << (2 * LABEL_BIT_WIDTH));
    ui b = 0; //skip impossible_code slice in g_lmtf_arr head
    while (b < k && g_lmtf_arr[b].code == impossible_code)
    {
        ++b;
    }
    ui idxg = b;
    for (ui ii = 0; idxg < k && ii < q_lmtf_arr_sz; ++ii)
    {
        ui qcode = q_lmtf_arr[ii].code;
        ui qcnt = q_lmtf_arr[ii].cnt;
        ui tmpcode;
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
                ui b = 0, e = q_lmtf_arr_sz - 1, idx;
                int new_e;

                ui ii = 0;
                //skip impossible code
                while (g_lmtf_arr[ii].code == impossible_code)
                {
                    ++ii;
                }
                //g_lmtf_arr[ii].code != impossible_code
                for (; ii < k; ++ii) //data_vertex in G记录的每个特征
                {
                    ui code = g_lmtf_arr[ii].code;
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

bool FilterVertices::LabelMotifFilter_1step(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count)
{
    if (!LabelMotifFilter(data_graph, query_graph, candidates, candidates_count)) //NLF
        return false;
    ui q_vertex_num = query_graph->getVerticesCount();
    for (ui q_v = 0; q_v < q_vertex_num; ++q_v)
    {
        ui in_count, out_count, bi_count;
        const ui *q_in_neighbors = query_graph->getVertexInNeighbors(q_v, in_count);
        const ui *q_out_neighbors = query_graph->getVertexOutNeighbors(q_v, out_count);
        const ui *q_bi_neighbors = query_graph->getVertexBiNeighbors(q_v, bi_count);
        ui candi_num = candidates_count[q_v];
        for (ui j = 0; j < candi_num; ++j)
        {
            ui d_v = candidates[q_v][j];
            if (d_v != INVALID_VERTEX_ID)
            {
                ui d_in_count, d_out_count, d_bi_count;
                const ui *d_in_neighbors = data_graph->getVertexInNeighbors(d_v, d_in_count);
                const ui *d_out_neighbors = data_graph->getVertexOutNeighbors(d_v, d_out_count);
                const ui *d_bi_neighbors = data_graph->getVertexBiNeighbors(d_v, d_bi_count);
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

bool FilterVertices::check_lmtf_cover(ui in_count, ui d_in_count, const ui *q_in_neighbors, const ui *d_in_neighbors, const Graph *query_graph, const Graph *data_graph)
{
    for (ui ii = 0; ii < in_count; ++ii) //for all q_v_n, find covers in d_v_n
    {
        ui q_v_n = q_in_neighbors[ii];
        const std::unordered_map<LabelID, ui> *q_in_map = query_graph->getVertexInNLF(q_v_n);
        const std::unordered_map<LabelID, ui> *q_out_map = query_graph->getVertexOutNLF(q_v_n);
        const std::unordered_map<LabelID, ui> *q_bi_map = query_graph->getVertexBiNLF(q_v_n);
        ui q_in_d = query_graph->getVertexInDegree(q_v_n);
        ui q_out_d = query_graph->getVertexOutDegree(q_v_n);
        ui q_bi_d = query_graph->getVertexBiDegree(q_v_n);
        LabelID q_v_n_label = query_graph->getVertexLabel(q_v_n);
        ui k = data_graph->getK();

        ui q_lmtf_arr_sz;
        const Lmtf *q_lmtf_arr = query_graph->getVertexLabelMotifCount(q_v_n, q_lmtf_arr_sz);

        ui jj;
        for (jj = 0; jj < d_in_count; ++jj)
        {
            ui d_v_n = d_in_neighbors[jj];
            if (data_graph->getVertexLabel(d_v_n) == q_v_n_label)
            {
#ifdef ONLINE_DEBUG
                std::cout << "q_v_n:" << q_v_n << "\nd_v_n:" << d_v_n << std::endl;
#endif //ONLINE_DEBUG
                if (((data_graph->getVertexInDegree(d_v_n)) >= q_in_d) && ((data_graph->getVertexOutDegree(d_v_n)) >= q_out_d) && ((data_graph->getVertexBiDegree(d_v_n)) >= q_bi_d))
                {

                    const std::unordered_map<LabelID, ui> *d_in_map = data_graph->getVertexInNLF(d_v_n);
                    const std::unordered_map<LabelID, ui> *d_out_map = data_graph->getVertexOutNLF(d_v_n);
                    const std::unordered_map<LabelID, ui> *d_bi_map = data_graph->getVertexBiNLF(d_v_n);
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
/*
[q_v特征中在G中找到的数目,q_v的总特征数,
记录为可以覆盖的q_v当中覆盖的q_v的特征,记录为可以覆盖的q_v当中q_v的总特征数]
为统计时间服务
*/
bool FilterVertices::LabelMotifFilter_collect_data_feature(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count, ui *feature)
{
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
        return false;

    ui query_vertex_num = query_graph->getVerticesCount();
    ui data_vertex_num = data_graph->getVerticesCount();

    /*
    motif filter
    */
    ui impossible_code = (1 << (2 * LABEL_BIT_WIDTH));

    VertexID *q_candidates;
    ui q_candidates_num;
#if DIRECTED_GRAPH == 1
    //label_motif_count_
    Lmtf *q_lmtf_arr, *g_lmtf_arr;
    ui q_lmtf_arr_sz;
    ui k = data_graph->getK();
    for (ui i = 0; i < query_vertex_num; ++i)
    {
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        q_lmtf_arr = query_graph->getVertexLabelMotifCount(i, q_lmtf_arr_sz);
        for (ui j = 0; j < q_candidates_num; ++j)
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
#endif // DIRECTED_GRAPH

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
bool FilterVertices::lmtf_arr_cover_collect_data_feature(const Lmtf *g_lmtf_arr, ui k, const Lmtf *q_lmtf_arr, ui q_lmtf_arr_sz, ui *feature)
{
    //q_lmtf_arr g_lmtf_arr: code ascending
    ui impossible_code = (1 << (2 * LABEL_BIT_WIDTH));
    ui b = 0; //skip impossible_code slice in g_lmtf_arr head
    while (b < k && g_lmtf_arr[b].code == impossible_code)
    {
        ++b;
    }
    ui idxg = b;

    ui cover_feature_cnt = 0;    //记录为可以覆盖的q_v当中覆盖的q_v的特征
    feature[1] += q_lmtf_arr_sz; //q_v的总特征数

    bool flag = 1;

    for (ui ii = 0; idxg < k && ii < q_lmtf_arr_sz; ++ii)
    {
        ui qcode = q_lmtf_arr[ii].code;
        ui qcnt = q_lmtf_arr[ii].cnt;
        ui tmpcode;
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
bool FilterVertices::LabelMotifFilter_limit(const Graph *data_graph, const Graph *query_graph, ui **&candidates, ui *&candidates_count)
{
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
        return false;

    ui query_vertex_num = query_graph->getVerticesCount();
    ui data_vertex_num = data_graph->getVerticesCount();

    /*
    motif filter
    */
    VertexID *q_candidates;
    ui q_candidates_num;

    //label_motif_map_
    std::map<ui, ui> *q_lmtf_map, *g_lmtf_map;
    for (ui i = 0; i < query_vertex_num; ++i)
    {
        q_candidates = candidates[i];
        q_candidates_num = candidates_count[i];
        q_lmtf_map = query_graph->getVertexLabelMotifMap(i);
        for (ui j = 0; j < q_candidates_num; ++j)
        {
            VertexID data_vertex = q_candidates[j];
            if (data_vertex != INVALID_VERTEX_ID)
            {
                g_lmtf_map = data_graph->getVertexLabelMotifMap(data_vertex);
                for (auto item : *q_lmtf_map)
                {
                    ui code = item.first;
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

//computeCandidateWithNLF
#if DIRECTED_GRAPH == 1
void FilterVertices::computeCandidateWithNLF_directed(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                                      ui &count, ui *buffer) //count buffer: candidates_count[query_vertex], candidates[query_vertex]
{
    LabelID label = query_graph->getVertexLabel(query_vertex);
    ui indegree = query_graph->getVertexInDegree(query_vertex);
    ui outdegree = query_graph->getVertexOutDegree(query_vertex);
    ui bidegree = query_graph->getVertexBiDegree(query_vertex);
#if OPTIMIZED_LABELED_GRAPH == 1
    const std::unordered_map<LabelID, ui> *query_vertex_in_nlf = query_graph->getVertexInNLF(query_vertex);
    const std::unordered_map<LabelID, ui> *query_vertex_out_nlf = query_graph->getVertexOutNLF(query_vertex);
    const std::unordered_map<LabelID, ui> *query_vertex_bi_nlf = query_graph->getVertexBiNLF(query_vertex);
#endif
    ui data_vertex_num;
    const ui *data_vertices = data_graph->getVerticesByLabel(label, data_vertex_num);
    count = 0;
    for (ui j = 0; j < data_vertex_num; ++j)
    {
        ui data_vertex = data_vertices[j];
        if ((data_graph->getVertexInDegree(data_vertex) >= indegree) && (data_graph->getVertexOutDegree(data_vertex) >= outdegree) && (data_graph->getVertexBiDegree(data_vertex) >= bidegree))
        {
            // NLF check
#if OPTIMIZED_LABELED_GRAPH == 1
            const std::unordered_map<LabelID, ui> *data_vertex_in_nlf = data_graph->getVertexInNLF(data_vertex);
            const std::unordered_map<LabelID, ui> *data_vertex_out_nlf = data_graph->getVertexOutNLF(data_vertex);
            const std::unordered_map<LabelID, ui> *data_vertex_bi_nlf = data_graph->getVertexBiNLF(data_vertex);
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
#else
            if (buffer != NULL)
            {
                buffer[count] = data_vertex;
            }
            count += 1;
#endif
        }
    }
}

#else
void FilterVertices::computeCandidateWithNLF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                             ui &count, ui *buffer) //count buffer: candidates_count[query_vertex], candidates[query_vertex]
{
    LabelID label = query_graph->getVertexLabel(query_vertex);
    ui degree = query_graph->getVertexDegree(query_vertex);
#if OPTIMIZED_LABELED_GRAPH == 1
    const std::unordered_map<LabelID, ui> *query_vertex_nlf = query_graph->getVertexNLF(query_vertex);
#endif
    ui data_vertex_num;
    const ui *data_vertices = data_graph->getVerticesByLabel(label, data_vertex_num);
    count = 0;
    for (ui j = 0; j < data_vertex_num; ++j)
    {
        ui data_vertex = data_vertices[j];
        if (data_graph->getVertexDegree(data_vertex) >= degree)
        {

            // NFL check
#if OPTIMIZED_LABELED_GRAPH == 1
            const std::unordered_map<LabelID, ui> *data_vertex_nlf = data_graph->getVertexNLF(data_vertex);

            if (data_vertex_nlf->size() >= query_vertex_nlf->size())
            {
                bool is_valid = true;

                for (auto element : *query_vertex_nlf)
                {
                    auto iter = data_vertex_nlf->find(element.first);
                    if (iter == data_vertex_nlf->end() || iter->second < element.second)
                    {
                        is_valid = false;
                        break;
                    }
                }

                if (is_valid) //succeed the NLF test, add to candi
                {
                    if (buffer != NULL)
                    {
                        buffer[count] = data_vertex;
                    }
                    count += 1;
                }
            }
#else
            if (buffer != NULL)
            {
                buffer[count] = data_vertex;
            }
            count += 1;
#endif
        }
    }
}

#endif //DIRECTED_GRAPH == 1

void FilterVertices::compactCandidates(ui **&candidates, ui *&candidates_count, ui query_vertex_num)
{
    for (ui i = 0; i < query_vertex_num; ++i)
    {
        VertexID query_vertex = i;
        ui next_position = 0;
        for (ui j = 0; j < candidates_count[query_vertex]; ++j)
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

bool FilterVertices::isCandidateSetValid(ui **&candidates, ui *&candidates_count, ui query_vertex_num)
{
    for (ui i = 0; i < query_vertex_num; ++i)
    {
        if (candidates_count[i] == 0)
            return false;
    }
    return true;
}

void FilterVertices::allocateBuffer(const Graph *data_graph, const Graph *query_graph, ui **&candidates,
                                    ui *&candidates_count)
{
    ui query_vertex_num = query_graph->getVerticesCount();
    ui candidates_max_num = data_graph->getGraphMaxLabelFrequency();

    candidates_count = new ui[query_vertex_num];
    memset(candidates_count, 0, sizeof(ui) * query_vertex_num);

    candidates = new ui *[query_vertex_num];

    for (ui i = 0; i < query_vertex_num; ++i)
    {
        candidates[i] = new ui[candidates_max_num];
    }
}
/*
q_lmtf_arr[b,e] 二分找code为code的，返回下标
如果找不到，返回大于其的最近
q_lmtf_arr已经按code升序，后面可能会出现长串的impossible_code
如果判断是impossible_code而调整e，则会int引用返回e
*/
ui FilterVertices::bi_search(Lmtf *q_lmtf_arr, ui ub, ui ue, ui code, int &new_e)
{
    ui impossible_code = (1 << (2 * LABEL_BIT_WIDTH));
    ui tmpcode;
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
void FilterVertices::print_candi(ui **candidates, ui *candidates_count, ui vertex_num)
{
    std::ofstream fout("candidates.txt");
    for (ui i = 0; i < vertex_num; ++i)
    {
        fout << i + 1;
        for (ui j = 0; j < candidates_count[i]; ++j)
        {
            fout << "\t" << candidates[i][j] + 1;
        }
        fout << "\n";
    }
    fout.close();
}

bool FilterVertices::arr_cover(const ui *darr, const ui *qarr, ui sz)
{
    for (ui i = 0; i < sz; ++i)
    {
        if (darr[i] < qarr[i])
        {
            return 0;
        }
    }
    return 1;
}

//dmap covers qmap
bool FilterVertices::map_cover(const std::unordered_map<LabelID, ui> *dmap, const std::unordered_map<LabelID, ui> *qmap)
{
#ifdef ONLINE_DEBUG
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
#endif //ONLINE_DEBUG
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

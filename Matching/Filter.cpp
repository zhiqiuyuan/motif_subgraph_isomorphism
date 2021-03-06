#include "Filter.h"
bool Filter::LDFFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count)
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

bool Filter::check_nlf_cover(unsigned in_count, unsigned d_in_count, const unsigned *q_in_neighbors, const unsigned *d_in_neighbors, const Graph *query_graph, const Graph *data_graph)
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

bool Filter::NLFFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count)
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

bool Filter::NLFFilter1step(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count)
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

bool Filter::GQLFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count)
{
    // Local refinement.
    //NLF???????????????GQL?????????????????????????????????lexicographic order?????????????????????
    if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
        return false;
    /*??????GQL????????????labelmotif_limit???
    if (!LabelMotifFilter_limit(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
        return false;
    */
    // Allocate buffer.
    unsigned query_vertex_num = query_graph->getVerticesCount();
    unsigned data_vertex_num = data_graph->getVerticesCount();

    bool **valid_candidates = new bool *[query_vertex_num]; //query_vertex_num * data_vertex_num
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        valid_candidates[i] = new bool[data_vertex_num];
        memset(valid_candidates[i], 0, sizeof(bool) * data_vertex_num);
    }

    unsigned query_graph_max_degree = query_graph->getGraphMaxSingleDegree();
    unsigned data_graph_max_degree = data_graph->getGraphMaxSingleDegree();

    //used for one u in q and one v in G, finding bi map between N(u) and N(v)
    int *left_to_right_offset = new int[query_graph_max_degree + 1]; //every vertex in N(u) has an entry, and an ending entry
    int *left_to_right_edges = new int[query_graph_max_degree * data_graph_max_degree];
    //every vertex in N(u) has some vmatches,???????????????????????????????????????vmatches???data_vertex_neighbors???????????????

    int *left_to_right_match = new int[query_graph_max_degree];
    int *right_to_left_match = new int[data_graph_max_degree];
    int *match_visited = new int[data_graph_max_degree + 1];  //N(V) visited?
    int *match_queue = new int[query_vertex_num];             //N(u) queue
    int *match_previous = new int[data_graph_max_degree + 1]; //for v' in N(v):previously matched u'

    // Record valid candidate vertices for each query vertex.
    //??????candidates???candidates_count???LDF NLF???q?????????u???????????????????????????????????????????????????
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        VertexID query_vertex = i;
        for (unsigned j = 0; j < candidates_count[query_vertex]; ++j)
        {
            VertexID data_vertex = candidates[query_vertex][j];
            valid_candidates[query_vertex][data_vertex] = true; //here, see valid_candidates a bool matrix
        }
    }

    // Global refinement.
    for (unsigned l = 0; l < 2; ++l) //k==2
    {
        for (unsigned i = 0; i < query_vertex_num; ++i) //??????q????????????????????????????????????????????????
        {
            VertexID query_vertex = i;
            for (unsigned j = 0; j < candidates_count[query_vertex]; ++j)
            {
                VertexID data_vertex = candidates[query_vertex][j]; //???data_vertex????????????????????????pruned

                if (data_vertex == INVALID_VERTEX_ID)
                    continue;

                //??????????????????N_in,N_out,N_bi(????????????),?????????????????? ??????:N(query_vertex)->N(data_vertex)
                const unsigned *q_in_neighbors, *q_out_neighbors, *q_bi_neighbors;
                const unsigned *d_in_neighbors, *d_out_neighbors, *d_bi_neighbors;
                unsigned q_in_degree, q_out_degree, q_bi_degree;
                unsigned d_in_degree, d_out_degree, d_bi_degree;
                q_in_neighbors = query_graph->getVertexInNeighbors(query_vertex, q_in_degree);
                q_out_neighbors = query_graph->getVertexOutNeighbors(query_vertex, q_out_degree);
                q_bi_neighbors = query_graph->getVertexBiNeighbors(query_vertex, q_bi_degree);
                d_in_neighbors = data_graph->getVertexInNeighbors(data_vertex, d_in_degree);
                d_out_neighbors = data_graph->getVertexOutNeighbors(data_vertex, d_out_degree);
                d_bi_neighbors = data_graph->getVertexBiNeighbors(data_vertex, d_bi_degree);
                if (checkInjection(q_bi_neighbors, d_bi_neighbors, q_bi_degree, d_bi_degree, valid_candidates,
                                   left_to_right_offset, left_to_right_edges, left_to_right_match,
                                   right_to_left_match, match_visited, match_queue, match_previous) &&
                    checkInjection(q_in_neighbors, d_in_neighbors, q_in_degree, d_in_degree, valid_candidates,
                                   left_to_right_offset, left_to_right_edges, left_to_right_match,
                                   right_to_left_match, match_visited, match_queue, match_previous) &&
                    checkInjection(q_out_neighbors, d_out_neighbors, q_out_degree, d_out_degree, valid_candidates,
                                   left_to_right_offset, left_to_right_edges, left_to_right_match,
                                   right_to_left_match, match_visited, match_queue, match_previous))
                {
                }
                else
                {
                    candidates[query_vertex][j] = INVALID_VERTEX_ID; //????????????candi???????????????????????????????????????
                    valid_candidates[query_vertex][data_vertex] = false;
                }
            }
        }
    }

    // Compact candidates.
    compactCandidates(candidates, candidates_count, query_vertex_num);

    // Release memory.
    for (unsigned i = 0; i < query_vertex_num; ++i)
    {
        delete[] valid_candidates[i];
    }
    delete[] valid_candidates;
    delete[] left_to_right_offset;
    delete[] left_to_right_edges;
    delete[] left_to_right_match;
    delete[] right_to_left_match;
    delete[] match_visited;
    delete[] match_queue;
    delete[] match_previous;

    return isCandidateSetValid(candidates, candidates_count, query_vertex_num);
}

bool Filter::CFLFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count,
                       unsigned *&order, TreeNode *&tree)
{
    allocateBuffer(data_graph, query_graph, candidates, candidates_count);
    int level_count;
    unsigned *level_offset;
    GenerateOrder::generateCFLFilterOrder(data_graph, query_graph, tree, order, level_count, level_offset);

    VertexID start_vertex = order[0];
    computeCandidateWithNLF(data_graph, query_graph, start_vertex, candidates_count[start_vertex], candidates[start_vertex]);

    unsigned *updated_flag = new unsigned[data_graph->getVerticesCount()];
    unsigned *flag = new unsigned[data_graph->getVerticesCount()];
    std::fill(flag, flag + data_graph->getVerticesCount(), 0);

    // Top-down generation.
    for (int i = 1; i < level_count; ++i)
    {
        // Forward generation.
        for (int j = level_offset[i]; j < level_offset[i + 1]; ++j)
        {
            VertexID query_vertex = order[j];
            TreeNode &node = tree[query_vertex];
            /*
            ?????????????????????????????? 
            1.in??????????????????out??????
            2.out??????????????????in??????
            3.bi??????????????????bi??????
            ?????????
            */
            generateCandidates(data_graph, query_graph, query_vertex, node, "bn", candidates, candidates_count, flag, updated_flag);
        }

        // Backward prune.
        for (int j = level_offset[i + 1] - 1; j >= level_offset[i]; --j)
        {
            VertexID query_vertex = order[j];
            TreeNode &node = tree[query_vertex];
            if (node.in_bn_count_ > 0 || node.out_bn_count_ > 0)
            {
                pruneCandidates(data_graph, query_graph, query_vertex, node, "fn", candidates, candidates_count, flag, updated_flag);
            }
        }
    }

    // Bottom-up refinement.
    for (int i = level_count - 2; i >= 0; --i)
    {
        for (int j = level_offset[i]; j < level_offset[i + 1]; ++j)
        {
            VertexID query_vertex = order[j];
            TreeNode &node = tree[query_vertex];

            if (node.in_under_level_count_ > 0 || node.out_under_level_count_ > 0)
            {
                //BFS??????????????????
                pruneCandidates(data_graph, query_graph, query_vertex, node, "under_level", candidates, candidates_count, flag, updated_flag);
            }
        }
    }

    compactCandidates(candidates, candidates_count, query_graph->getVerticesCount());

    delete[] updated_flag;
    delete[] flag;
    return isCandidateSetValid(candidates, candidates_count, query_graph->getVerticesCount());
}

bool Filter::DPisoFilter(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates, unsigned *&candidates_count,
                         unsigned *&order, TreeNode *&tree)
{
    if (!LDFFilter(data_graph, query_graph, candidates, candidates_count))
        return false;
    /* ?????????ordering method?????????DPiso?????????????????????????????????find tree-like path, build weight array?????????????????????????????????
    ??????????????????enumerate??????????????????extendable???????????????????????????filter???????????????BFS order??? */
    GenerateOrder::generateDPisoFilterOrder(data_graph, query_graph, tree, order);

    unsigned query_vertices_num = query_graph->getVerticesCount();
    unsigned *updated_flag = new unsigned[data_graph->getVerticesCount()];
    unsigned *flag = new unsigned[data_graph->getVerticesCount()];
    std::fill(flag, flag + data_graph->getVerticesCount(), 0);

    // The number of refinement is k. According to the original paper, we set k as 3.
    for (unsigned k = 0; k < 3; ++k)
    {
        if (k % 2 == 0)
        {
            //along $, ??????
            for (int i = 1; i < query_vertices_num; ++i)
            {
                VertexID query_vertex = order[i];
                TreeNode &node = tree[query_vertex];
                pruneCandidates(data_graph, query_graph, query_vertex, node, "bn", candidates, candidates_count, flag, updated_flag);
            }
        }
        else
        {
            //along reverse $, ??????
            for (int i = query_vertices_num - 2; i >= 0; --i)
            {
                VertexID query_vertex = order[i];
                TreeNode &node = tree[query_vertex];
                pruneCandidates(data_graph, query_graph, query_vertex, node, "fn", candidates, candidates_count, flag, updated_flag);
            }
        }
    }

    compactCandidates(candidates, candidates_count, query_graph->getVerticesCount());

    delete[] updated_flag;
    delete[] flag;
    return isCandidateSetValid(candidates, candidates_count, query_graph->getVerticesCount());
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
/* WARNNING:?????????????????????????????????????????????k????????????????????????????????????????????????
1.vertex label considered
2.data_graph,query_graph: motif structure already loaded
3.data_graph: k already loaded
4.will allocate space for candidates and candidates_count
*/
bool Filter::LabelMotifFilter(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, std::string firstStageFilterKind)
{
    if (firstStageFilterKind == "" || firstStageFilterKind == "nlf")
    {
        if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
            return false;
    }
    else if (firstStageFilterKind == "gql")
    {
        if (!GQLFilter(data_graph, query_graph, candidates, candidates_count))
            return false;
    }
    else if (firstStageFilterKind == "cfl")
    {
        //CFLFilter??????DPisoFilter??????????????????weakBFS???????????????????????????
        unsigned *order;
        TreeNode *tree;
        bool re = CFLFilter(data_graph, query_graph, candidates, candidates_count, order, tree);
        delete[] order;
        delete[] tree;
        if (re == 0)
            return false;
    }
    else if (firstStageFilterKind == "dpiso")
    {
        //CFLFilter??????DPisoFilter??????????????????weakBFS???????????????????????????
        unsigned *order;
        TreeNode *tree;
        bool re = DPisoFilter(data_graph, query_graph, candidates, candidates_count, order, tree);
        delete[] order;
        delete[] tree;
        if (re == 0)
            return false;
    }
    else
    {
        std::cout << "[WARNNING:] not support firstStageFilterKind:" << firstStageFilterKind << " use NLFFilter." << std::endl;
        if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
            return false;
    }

    unsigned query_vertex_num = query_graph->getVerticesCount();
    unsigned data_vertex_num = data_graph->getVerticesCount();

    /*
    motif filter
    */
    unsigned impossible_code = (1 << (2 * LABEL_BIT_WIDTH));

    VertexID *q_candidates;
    unsigned q_candidates_num;

    //label_motif_count_
    /* ?????????????????????????????????????????????k???
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

//???????????????
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
                for (; ii < k; ++ii) //data_vertex in G?????????????????????
                {
                    unsigned code = g_lmtf_arr[ii].code;
                    if (code == impossible_code)
                    {
                        break; //??????impossible code,skip
                    }
                    idx = bi_search(q_lmtf_arr, b, e, code, new_e); //[b,e]search
                    if (idx > e || idx < b || new_e == -1)
                    { //?????????????????????????????????????????????
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

/* WARNNING:???????????????????????????LabelMotifFilter????????????????????????????????????k????????????????????????????????????????????????
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

/* WARNNING:??????????????????????????????????????????????????????k????????????????????????????????????????????????
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

/* WARNNING:??????????????????????????????????????????????????????k????????????????????????????????????????????????
[q_v????????????G??????????????????,q_v???????????????,
????????????????????????q_v???????????????q_v?????????,????????????????????????q_v??????q_v???????????????]
?????????????????????
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
[q_v????????????G???????????????cnt!=1??????,q_v???????????????,
????????????????????????q_v??????q_v????????????G???????????????cnt!=1??????,????????????????????????q_v??????q_v???????????????]
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

    unsigned cover_feature_cnt = 0; //????????????????????????q_v???????????????q_v?????????
    feature[1] += q_lmtf_arr_sz;    //q_v???????????????

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
                    cover_feature_cnt++; //q_v?????????cnt!=1??????G??????????????????
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
    feature[0] += cover_feature_cnt; //q_v????????????G??????????????????
    if (flag)
    {
        //q_v?????????????????????
        feature[2] += cover_feature_cnt;
        feature[3] += q_lmtf_arr_sz;
    }
    return flag;
}

#if LABEL_MOTIF_LIMIT == 1
bool Filter::LabelMotifFilter_limit(const MotifG *data_graph, const MotifQ *query_graph, unsigned **&candidates, unsigned *&candidates_count, std::string firstStageFilterKind)
{
    if (firstStageFilterKind == "" || firstStageFilterKind == "nlf")
    {
        if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
            return false;
    }
    else if (firstStageFilterKind == "gql")
    {
        if (!GQLFilter(data_graph, query_graph, candidates, candidates_count))
            return false;
    }
    else if (firstStageFilterKind == "cfl")
    {
        //CFLFilter??????DPisoFilter??????????????????weakBFS???????????????????????????
        unsigned *order;
        TreeNode *tree;
        bool re = CFLFilter(data_graph, query_graph, candidates, candidates_count, order, tree);
        delete[] order;
        delete[] tree;
        if (re == 0)
            return false;
    }
    else if (firstStageFilterKind == "dpiso")
    {
        //CFLFilter??????DPisoFilter??????????????????weakBFS???????????????????????????
        unsigned *order;
        TreeNode *tree;
        bool re = DPisoFilter(data_graph, query_graph, candidates, candidates_count, order, tree);
        delete[] order;
        delete[] tree;
        if (re == 0)
            return false;
    }
    else
    {
        std::cout << "[WARNNING:] not support firstStageFilterKind:" << firstStageFilterKind << " use NLFFilter." << std::endl;
        if (!NLFFilter(data_graph, query_graph, candidates, candidates_count)) //LDF NLF
            return false;
    }

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

double Filter::LDFFilter_AveCandiScale(const Graph *data_graph, const Graph *query_graph)
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
    //printCandidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}

double Filter::NLFFilter_AveCandiScale(const Graph *data_graph, Graph *query_graph)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();
    //?????????????????????????????????
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
    //printCandidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}

double Filter::NLFFilter1step_AveCandiScale(const Graph *data_graph, Graph *query_graph)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();
    //?????????????????????????????????
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
    //printCandidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}

double Filter::GQLFilter_AveCandiScale(const Graph *data_graph, Graph *query_graph)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();
    //?????????????????????????????????
    query_graph->BuildNLF();
    /* ??????GQL????????????labelmotif_limit????????????
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
    */

    if (Filter::GQLFilter(data_graph, query_graph, candidates, candidates_count))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    //printCandidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}

double Filter::CFLFilter_AveCandiScale(const Graph *data_graph, Graph *query_graph)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();
    //?????????????????????????????????
    query_graph->BuildNLF();
    /* ??????CFL????????????labelmotif_limit????????????
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
    */

    //CFLFilter??????????????????weakBFS???????????????????????????
    unsigned *order;
    TreeNode *tree;
    if (Filter::CFLFilter(data_graph, query_graph, candidates, candidates_count, order, tree))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    //printCandidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    //??????filter???????????????????????????????????????????????????????????????
    delete[] tree;
    delete[] order;

    for (unsigned i = 0; i < qVScale; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    return new_candiScale;
}

double Filter::DPisoFilter_AveCandiScale(const Graph *data_graph, Graph *query_graph)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();
    //?????????????????????????????????
    /* ??????DPiso????????????labelmotif_limit????????????
    query_graph->buildNLF();
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
    */

    //?????????DPisoFilter??????????????????weakBFS???????????????????????????
    unsigned *order;
    TreeNode *tree;
    if (Filter::DPisoFilter(data_graph, query_graph, candidates, candidates_count, order, tree))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    //printCandidates(candidates, candidates_count, qVScale);
#endif //#if STEP_DEBUG==1

    //??????filter???????????????????????????????????????????????????????????????
    delete[] tree;
    delete[] order;

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

    //?????????????????????????????????
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

    /* WARNNING: ?????????1???????????????NLF*/
    if (Filter::TopoMotifFilter(data_graph, query_graph, candidates, candidates_count, 1))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    //printCandidates(candidates, candidates_count, qVScale);
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

    //?????????????????????????????????
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

    /* WARNNING: ?????????1???????????????NLF*/
    if (Filter::TopoMotifFilter1step(data_graph, query_graph, candidates, candidates_count, 1))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    //printCandidates(candidates, candidates_count, qVScale);
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

/* WARNNING: ??????????????????labelMotifFilter???????????????
*/
#if LABEL_MOTIF_ENABLE == 1
#if LABEL_MOTIF_LIMIT == 1
double Filter::LabelMotifFilter_limit_AveCandiScale(const MotifG *data_graph, MotifQ *query_graph, std::string firstStageFilterKind, std::string filename)
{
    unsigned **candidates = 0;
    unsigned *candidates_count = 0;
    double new_candiScale;
    unsigned qVScale = query_graph->getVerticesCount();

    //?????????????????????????????????
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

    if (Filter::LabelMotifFilter_limit(data_graph, query_graph, candidates, candidates_count, firstStageFilterKind))
    {
        new_candiScale = get_average(candidates_count, qVScale);
    }
    else
    {
        new_candiScale = 0;
    }

#if STEP_DEBUG == 1
    //printCandidates(candidates, candidates_count, qVScale);
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

void Filter::computeCandidateWithLDF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                     unsigned &count, unsigned *buffer)
{
    LabelID label = query_graph->getVertexLabel(query_vertex);
    unsigned indegree = query_graph->getVertexInDegree(query_vertex);
    unsigned outdegree = query_graph->getVertexOutDegree(query_vertex);
    unsigned bidegree = query_graph->getVertexBiDegree(query_vertex);
    count = 0;
    unsigned data_vertex_num;
    const unsigned *data_vertices = data_graph->getVerticesByLabel(label, data_vertex_num);

    if (buffer == NULL)
    {
        for (unsigned i = 0; i < data_vertex_num; ++i)
        {
            VertexID v = data_vertices[i];
            if (data_graph->getVertexInDegree(v) >= indegree &&
                data_graph->getVertexOutDegree(v) >= outdegree &&
                data_graph->getVertexBiDegree(v) >= bidegree)
            {
                count += 1;
            }
        }
    }
    else
    {
        for (unsigned i = 0; i < data_vertex_num; ++i)
        {
            VertexID v = data_vertices[i];
            if (data_graph->getVertexInDegree(v) >= indegree &&
                data_graph->getVertexOutDegree(v) >= outdegree &&
                data_graph->getVertexBiDegree(v) >= bidegree)
            {
                buffer[count++] = v;
            }
        }
    }
}

void Filter::computeCandidateWithNLF(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
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

/*
q_lmtf_arr[b,e] ?????????code???code??????????????????
??????????????????????????????????????????
q_lmtf_arr?????????code???????????????????????????????????????impossible_code
???????????????impossible_code?????????e?????????int????????????e
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

/*
n: |N(u)|
match\row_match: 1.???????????????query_vertex?????????data_vertex??????????????????
                             [0,n)           [0,m)
                 2.match:q->G  row_match:G->q
*/
void Filter::old_cheap(int *col_ptrs, int *col_ids, int *match, int *row_match, int n, int m)
{
    int ptr;
    int i = 0;
    for (; i < n; i++)
    {
        //i in q, r_id in G
        int s_ptr = col_ptrs[i];
        int e_ptr = col_ptrs[i + 1];
        for (ptr = s_ptr; ptr < e_ptr; ptr++)
        {
            int r_id = col_ids[ptr]; //vertex in N(v)??????????????????vertex???N(vertex)?????????????????????match???row_match???????????????????????????????????????????????????
            if (row_match[r_id] == -1)
            {
                match[i] = r_id;
                row_match[r_id] = i;
                break; //???????????????match??????????????????
            }
        }
    }
}

/*
 -----N(v)----> m
|
N(u) n
|
v

match: N(u)->N(v)
row_match: N(v)->N(u)

visited: for N(v)
queue: for N(u)
previous: for N(v) //for v' in N(v): record previously matched u'

m: size of N(v)
n: size of N(u)
*/
void Filter::match_bfs(int *col_ptrs, int *col_ids, int *match, int *row_match, int *visited,
                       int *queue, int *previous, int n, int m)
{
    int queue_ptr, queue_front, ptr, next_augment_no, i, j, queue_size,
        row, col, temp, eptr;

    old_cheap(col_ptrs, col_ids, match, row_match, n, m);
    //?????????match???????????????q??????u?????????????????????match???v?????????match??????????????????u????????????????????????
    //????????????????????? ???????????????????????????u??????????????????????????????v?????????v???u???????????????v???????????????????????????????????????v????????????u???
    /*
    match\row_match: 1.??????????????? query_vertex?????? ??? data_vertex?????? ????????????
                                    [0,n)            [0,m)
                 2.match:q->G  row_match:G->q
    */

    memset(visited, 0, sizeof(int) * m);

    next_augment_no = 1;    //next_augment_number??????1??????
    for (i = 0; i < n; i++) //for i in N(u)?????????????????????N(v)??????match????????????
    {
        if (match[i] == -1 && col_ptrs[i] != col_ptrs[i + 1])
        //N(v)??????i??????map?????????????????????????????????????????????map???N(u)????????????
        //?????????map???N(v)????????????????????????map???N(u)?????????;?????????????????????map???N(v)???
        {
            queue[0] = i;
            queue_ptr = 0;
            queue_size = 1;

            while (queue_size > queue_ptr) //????????????
            {
                queue_front = queue[queue_ptr++];
                //???queue_front?????????map???N(v)???
                eptr = col_ptrs[queue_front + 1];
                for (ptr = col_ptrs[queue_front]; ptr < eptr; ptr++)
                {
                    row = col_ids[ptr];
                    temp = visited[row]; //visited: for N(v)

                    if (temp != next_augment_no && temp != -1) //row??????????????????
                    {
                        previous[row] = queue_front;
                        //previous??????N(v)???????????????map??????N(u)???
                        visited[row] = next_augment_no;

                        col = row_match[row]; //row?????????map??????N(u)??????

                        if (col == -1) //row?????????match?????????N(u)??????u'
                        {
                            // Find an augmenting path. Then, trace back and modify the augmenting path.
                            while (row != -1)
                            {                        //row:in N(v)???????????????previous[row]??????N(u)?????????match
                                col = previous[row]; // in N(u)
                                temp = match[col];   //col?????????map??????N(v)??????
                                match[col] = row;    //??????col?????????match???row
                                row_match[row] = col;
                                row = temp; //col?????????map??????N(v)??????
                            }
                            //row==-1???match[col]==-1???????????????????????????????????????????????????map???
                            next_augment_no++;
                            queue_size = 0;
                            break;
                        }
                        else
                        {
                            // Continue to construct the match.
                            queue[queue_size++] = col; //??????
                        }
                    }
                }
            }

            if (match[i] == -1) //i in N(u)???????????????match
            {
                for (j = 1; j < queue_size; j++)
                {
                    visited[match[queue[j]]] = -1;
                }
            }
        }
    }
}

/*
???????????????????????? ??????:N(query_vertex)->N(data_vertex)

Construct the bipartite graph between N(query_vertex) and N(data_vertex)???
    ??????query_vertex???????????????query_vertex_neighbor???
        ??????data_vertex???????????????data_vertex_neighbor????????????query_vertex_neighbor??????????????????????????????bipartite?????????edges???

left:q  right:G
*/
bool Filter::checkInjection(const unsigned *query_vertex_neighbors, const unsigned *data_vertex_neighbors, unsigned left_partition_size, unsigned right_partition_size,
                            bool **valid_candidates, int *left_to_right_offset, int *left_to_right_edges,
                            int *left_to_right_match, int *right_to_left_match, int *match_visited,
                            int *match_queue, int *match_previous)
{
    // Construct the bipartite graph between N(query_vertex) and N(data_vertex)
    unsigned edge_count = 0; //csr: left_to_right_offset,left_to_right_edges
    for (int i = 0; i < left_partition_size; ++i)
    {
        VertexID query_vertex_neighbor = query_vertex_neighbors[i];
        left_to_right_offset[i] = edge_count;

        for (int j = 0; j < right_partition_size; ++j) //data_vertex_neighbors num
        {
            VertexID data_vertex_neighbor = data_vertex_neighbors[j];

            /*
            here!
            */
            if (valid_candidates[query_vertex_neighbor][data_vertex_neighbor])
            {
                left_to_right_edges[edge_count++] = j;
            }
        }
    }
    left_to_right_offset[left_partition_size] = edge_count;

    //set -1???????????????1???
    memset(left_to_right_match, -1, left_partition_size * sizeof(int));
    memset(right_to_left_match, -1, right_partition_size * sizeof(int));

    //????????????left_to_right_match
    Filter::match_bfs(left_to_right_offset, left_to_right_edges, left_to_right_match, right_to_left_match,
                      match_visited, match_queue, match_previous, left_partition_size, right_partition_size);
    for (int i = 0; i < left_partition_size; ++i)
    {
        if (left_to_right_match[i] == -1)
            return false;
    }

    return true;
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

void Filter::allocateBuffer(const Graph *data_graph, const Graph *query_graph, unsigned **&candidates,
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
node???pivot_kind????????????
1.in??????????????????out??????
2.out??????????????????in??????
3.bi??????????????????bi??????
????????????????????????candi

pivot_kind: bn, fn, under_level
*/
void Filter::generateCandidates(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                                TreeNode &node, std::string pivot_kind, VertexID **candidates,
                                unsigned *candidates_count, unsigned *flag, unsigned *updated_flag)
{
    unsigned pivot_vertices_count;
    const unsigned *pivot_vertices;

    LabelID query_vertex_label = query_graph->getVertexLabel(query_vertex);
    unsigned query_vertex_degree = query_graph->getVertexDegree(query_vertex);
    unsigned count = 0;
    unsigned updated_flag_count = 0;

    /* ???flag???updated_flag
    flag[v]: ??????v???query_vertex???????????????????????????????????????????????????????????????==??????????????????????????????v???????????????????????????
    updated_flag: query_vertex???????????????????????????????????????????????????pass LDF???????????????????????????????????????????????????????????????????????????????????????
    */
    //in??????????????????out??????
    if (pivot_kind == "bn")
    {
        pivot_vertices = node.in_bn_;
        pivot_vertices_count = node.in_bn_count_;
    }
    else if (pivot_kind == "fn")
    {
        pivot_vertices = node.in_fn_;
        pivot_vertices_count = node.in_fn_count_;
    }
    else if (pivot_kind == "under_level")
    {
        pivot_vertices = node.in_under_level_;
        pivot_vertices_count = node.in_under_level_count_;
    }
    else
    {
        std::cout << __FILE__ << " " << __LINE__ << "\n";
        std::cout << "unsupported pivot kind: " << pivot_kind << std::endl;
        return;
    }
    for (unsigned i = 0; i < pivot_vertices_count; ++i)
    {
        VertexID pivot_vertex = pivot_vertices[i];
        for (unsigned j = 0; j < candidates_count[pivot_vertex]; ++j)
        {
            VertexID v = candidates[pivot_vertex][j];

            if (v == INVALID_VERTEX_ID)
                continue;
            unsigned v_nbrs_count;
            const VertexID *v_nbrs = data_graph->getVertexOutNeighbors(v, v_nbrs_count);

            for (unsigned k = 0; k < v_nbrs_count; ++k)
            {
                VertexID v_nbr = v_nbrs[k];
                LabelID v_nbr_label = data_graph->getVertexLabel(v_nbr);
                unsigned v_nbr_degree = data_graph->getVertexDegree(v_nbr);

                //??????v_nbr??????????????????????????????????????????????????????????????????????????????pass LDF??????????????????
                if (flag[v_nbr] == count && v_nbr_label == query_vertex_label && v_nbr_degree >= query_vertex_degree)
                {
                    flag[v_nbr] += 1;

                    if (count == 0) //???????????????in?????????????????????out??????????????????pass LDF???
                    {
                        updated_flag[updated_flag_count++] = v_nbr;
                    }
                }
            }
        }

        count += 1;
    }

    //out??????????????????in??????
    if (pivot_kind == "bn")
    {
        pivot_vertices = node.out_bn_;
        pivot_vertices_count = node.out_bn_count_;
    }
    else if (pivot_kind == "fn")
    {
        pivot_vertices = node.out_fn_;
        pivot_vertices_count = node.out_fn_count_;
    }
    else
    {
        pivot_vertices = node.out_under_level_;
        pivot_vertices_count = node.out_under_level_count_;
    }
    for (unsigned i = 0; i < pivot_vertices_count; ++i)
    {
        VertexID pivot_vertex = pivot_vertices[i];
        for (unsigned j = 0; j < candidates_count[pivot_vertex]; ++j)
        {
            VertexID v = candidates[pivot_vertex][j];

            if (v == INVALID_VERTEX_ID)
                continue;
            unsigned v_nbrs_count;
            const VertexID *v_nbrs = data_graph->getVertexInNeighbors(v, v_nbrs_count);

            for (unsigned k = 0; k < v_nbrs_count; ++k)
            {
                VertexID v_nbr = v_nbrs[k];
                LabelID v_nbr_label = data_graph->getVertexLabel(v_nbr);
                unsigned v_nbr_degree = data_graph->getVertexDegree(v_nbr);

                //??????v_nbr??????????????????????????????????????????????????????????????????????????????pass LDF??????????????????
                if (flag[v_nbr] == count && v_nbr_label == query_vertex_label && v_nbr_degree >= query_vertex_degree)
                {
                    flag[v_nbr] += 1;

                    if (count == 0) //???????????????in?????????????????????out??????????????????pass LDF???
                    {
                        updated_flag[updated_flag_count++] = v_nbr;
                    }
                }
            }
        }

        count += 1;
    }

    //bi??????????????????bi??????
    if (pivot_kind == "bn")
    {
        pivot_vertices = node.bi_bn_;
        pivot_vertices_count = node.bi_bn_count_;
    }
    else if (pivot_kind == "fn")
    {
        pivot_vertices = node.bi_fn_;
        pivot_vertices_count = node.bi_fn_count_;
    }
    else
    {
        pivot_vertices = node.bi_under_level_;
        pivot_vertices_count = node.bi_under_level_count_;
    }
    for (unsigned i = 0; i < pivot_vertices_count; ++i)
    {
        VertexID pivot_vertex = pivot_vertices[i];
        for (unsigned j = 0; j < candidates_count[pivot_vertex]; ++j)
        {
            VertexID v = candidates[pivot_vertex][j];

            if (v == INVALID_VERTEX_ID)
                continue;
            unsigned v_nbrs_count;
            const VertexID *v_nbrs = data_graph->getVertexBiNeighbors(v, v_nbrs_count);

            for (unsigned k = 0; k < v_nbrs_count; ++k)
            {
                VertexID v_nbr = v_nbrs[k];
                LabelID v_nbr_label = data_graph->getVertexLabel(v_nbr);
                unsigned v_nbr_degree = data_graph->getVertexDegree(v_nbr);

                //??????v_nbr??????????????????????????????????????????????????????????????????????????????pass LDF??????????????????
                if (flag[v_nbr] == count && v_nbr_label == query_vertex_label && v_nbr_degree >= query_vertex_degree)
                {
                    flag[v_nbr] += 1;

                    if (count == 0) //???????????????in?????????????????????out??????????????????pass LDF???
                    {
                        updated_flag[updated_flag_count++] = v_nbr;
                    }
                }
            }
        }

        count += 1;
    }

    //NLF??????candidates
    const std::unordered_map<LabelID, unsigned> *q_in_nlf = query_graph->getVertexInNLF(query_vertex);
    const std::unordered_map<LabelID, unsigned> *q_out_nlf = query_graph->getVertexOutNLF(query_vertex);
    const std::unordered_map<LabelID, unsigned> *q_bi_nlf = query_graph->getVertexBiNLF(query_vertex);
    for (unsigned i = 0; i < updated_flag_count; ++i)
    {
        VertexID v = updated_flag[i];
        if (flag[v] == count) //v????????????
        {
            // NLF filter.
            if (map_cover(data_graph->getVertexBiNLF(v), q_bi_nlf) && map_cover(data_graph->getVertexInNLF(v), q_in_nlf) && map_cover(data_graph->getVertexOutNLF(v), q_out_nlf))
            {
                candidates[query_vertex][candidates_count[query_vertex]++] = v;
            }
        }
    }

    //???flag???????????????0????????????flag++????????????updated_flag????????????????????????????????????????????????????????????????????????????????????xxx??????????????????????????????????????????????????????????????????????????????????????????flag==count?????????????????????????????????flag???
    for (unsigned i = 0; i < updated_flag_count; ++i)
    {
        unsigned v = updated_flag[i];
        flag[v] = 0;
    }
}

/* 
???node???pivot_kind????????????
1.in??????????????????out??????
2.out??????????????????in??????
3.bi??????????????????bi??????
???????????????prune C(query_vertex):????????????????????????????????????

pivot_kind: bn, fn, under_level
*/
void Filter::pruneCandidates(const Graph *data_graph, const Graph *query_graph, VertexID query_vertex,
                             TreeNode &node, std::string pivot_kind, VertexID **candidates,
                             unsigned *candidates_count, unsigned *flag, unsigned *updated_flag)
{
    unsigned pivot_vertices_count;
    const unsigned *pivot_vertices;

    LabelID query_vertex_label = query_graph->getVertexLabel(query_vertex);
    unsigned query_vertex_degree = query_graph->getVertexDegree(query_vertex);

    unsigned count = 0;
    unsigned updated_flag_count = 0;

    //???????????????????????????flag==count????????????????????????
    //in??????????????????out??????
    if (pivot_kind == "bn")
    {
        pivot_vertices = node.in_bn_;
        pivot_vertices_count = node.in_bn_count_;
    }
    else if (pivot_kind == "fn")
    {
        pivot_vertices = node.in_fn_;
        pivot_vertices_count = node.in_fn_count_;
    }
    else if (pivot_kind == "under_level")
    {
        pivot_vertices = node.in_under_level_;
        pivot_vertices_count = node.in_under_level_count_;
    }
    else
    {
        std::cout << __FILE__ << " " << __LINE__ << "\n";
        std::cout << "unsupported pivot kind: " << pivot_kind << std::endl;
        return;
    }
    for (unsigned i = 0; i < pivot_vertices_count; ++i)
    {
        VertexID pivot_vertex = pivot_vertices[i];
        for (unsigned j = 0; j < candidates_count[pivot_vertex]; ++j)
        {
            VertexID v = candidates[pivot_vertex][j];

            if (v == INVALID_VERTEX_ID)
                continue;
            unsigned v_nbrs_count;
            const VertexID *v_nbrs = data_graph->getVertexOutNeighbors(v, v_nbrs_count);

            for (unsigned k = 0; k < v_nbrs_count; ++k)
            {
                VertexID v_nbr = v_nbrs[k];
                LabelID v_nbr_label = data_graph->getVertexLabel(v_nbr);
                unsigned v_nbr_degree = data_graph->getVertexDegree(v_nbr);

                //??????v_nbr??????????????????????????????????????????????????????????????????????????????pass LDF??????????????????
                if (flag[v_nbr] == count && v_nbr_label == query_vertex_label && v_nbr_degree >= query_vertex_degree)
                {
                    flag[v_nbr] += 1;

                    if (count == 0) //???????????????in?????????????????????out??????????????????pass LDF???
                    {
                        updated_flag[updated_flag_count++] = v_nbr;
                    }
                }
            }
        }

        count += 1;
    }

    //out??????????????????in??????
    if (pivot_kind == "bn")
    {
        pivot_vertices = node.out_bn_;
        pivot_vertices_count = node.out_bn_count_;
    }
    else if (pivot_kind == "fn")
    {
        pivot_vertices = node.out_fn_;
        pivot_vertices_count = node.out_fn_count_;
    }
    else
    {
        pivot_vertices = node.out_under_level_;
        pivot_vertices_count = node.out_under_level_count_;
    }
    for (unsigned i = 0; i < pivot_vertices_count; ++i)
    {
        VertexID pivot_vertex = pivot_vertices[i];
        for (unsigned j = 0; j < candidates_count[pivot_vertex]; ++j)
        {
            VertexID v = candidates[pivot_vertex][j];

            if (v == INVALID_VERTEX_ID)
                continue;
            unsigned v_nbrs_count;
            const VertexID *v_nbrs = data_graph->getVertexInNeighbors(v, v_nbrs_count);

            for (unsigned k = 0; k < v_nbrs_count; ++k)
            {
                VertexID v_nbr = v_nbrs[k];
                LabelID v_nbr_label = data_graph->getVertexLabel(v_nbr);
                unsigned v_nbr_degree = data_graph->getVertexDegree(v_nbr);

                //??????v_nbr??????????????????????????????????????????????????????????????????????????????pass LDF??????????????????
                if (flag[v_nbr] == count && v_nbr_label == query_vertex_label && v_nbr_degree >= query_vertex_degree)
                {
                    flag[v_nbr] += 1;

                    if (count == 0) //???????????????in?????????????????????out??????????????????pass LDF???
                    {
                        updated_flag[updated_flag_count++] = v_nbr;
                    }
                }
            }
        }

        count += 1;
    }

    //bi??????????????????bi??????
    if (pivot_kind == "bn")
    {
        pivot_vertices = node.bi_bn_;
        pivot_vertices_count = node.bi_bn_count_;
    }
    else if (pivot_kind == "fn")
    {
        pivot_vertices = node.bi_fn_;
        pivot_vertices_count = node.bi_fn_count_;
    }
    else
    {
        pivot_vertices = node.bi_under_level_;
        pivot_vertices_count = node.bi_under_level_count_;
    }
    for (unsigned i = 0; i < pivot_vertices_count; ++i)
    {
        VertexID pivot_vertex = pivot_vertices[i];
        for (unsigned j = 0; j < candidates_count[pivot_vertex]; ++j)
        {
            VertexID v = candidates[pivot_vertex][j];

            if (v == INVALID_VERTEX_ID)
                continue;
            unsigned v_nbrs_count;
            const VertexID *v_nbrs = data_graph->getVertexBiNeighbors(v, v_nbrs_count);

            for (unsigned k = 0; k < v_nbrs_count; ++k)
            {
                VertexID v_nbr = v_nbrs[k];
                LabelID v_nbr_label = data_graph->getVertexLabel(v_nbr);
                unsigned v_nbr_degree = data_graph->getVertexDegree(v_nbr);

                //??????v_nbr??????????????????????????????????????????????????????????????????????????????pass LDF??????????????????
                if (flag[v_nbr] == count && v_nbr_label == query_vertex_label && v_nbr_degree >= query_vertex_degree)
                {
                    flag[v_nbr] += 1;

                    if (count == 0) //???????????????in?????????????????????out??????????????????pass LDF???
                    {
                        updated_flag[updated_flag_count++] = v_nbr;
                    }
                }
            }
        }

        count += 1;
    }

    //prune
    for (unsigned i = 0; i < candidates_count[query_vertex]; ++i)
    {
        unsigned v = candidates[query_vertex][i];
        if (v == INVALID_VERTEX_ID)
            continue;

        if (flag[v] != count) //???????????????
        {
            candidates[query_vertex][i] = INVALID_VERTEX_ID;
        }
    }

    //???flag???????????????0
    for (unsigned i = 0; i < updated_flag_count; ++i)
    {
        unsigned v = updated_flag[i];
        flag[v] = 0;
    }
}

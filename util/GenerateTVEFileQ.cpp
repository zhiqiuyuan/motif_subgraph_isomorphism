#include "GenerateTVEFileQ.h"

#if COLLECT_Q_FEATURE == 1
std::set<unsigned> data_vertices_in_query_graph;
std::set<std::string> query_graph_kind; //vertexId in query_graph coded into string; want to see how many kind
#endif                                  //COLLECT_Q_FEATURE

/* 从in out邻居（即所有邻居）中随机选取下一个顶点
 * dfs from start
 * curr_qcnt:for file name
 * dest_dir: write to dest_dir/query_dense_vcnt_currqcnt.graph
 * file format: t v e
 * return whether found one
 * degree_threshold: some data_graph are too sparse that cannot find dense subgraph whose average degree>=3, some adjust it
 * 
 * WRITE_TO_FILE_DEBUG==0则以t v e格式写入文件，==1则以便于debug的格式写入文件dest_dir/query_dense_vcnt_currqcnt.graph
 * COLLECT_Q_FEATURE==1则会在生成查询图同时统计查询图特征，写入dest_dir的../properties_of_query_graph.txt
 */
bool gen_qgraph_from_start(unsigned start, unsigned vcnt_required, unsigned curr_qcnt, unsigned degree_threshold, std::string dest_dir, QGraph *currq, Graph *dg)
{
    bool isSparse = currq->isSparse;
#if STEP_DEBUG == 1
    std::cout << "v" << start + 1 << " ";
    //std::cout << "visited:v" << start + 1 << " curr_vcnt:" << currq->getVerticesCount() << " vcnt_required:" << vcnt_required << std::endl;
#endif //STEP_DEBUG
    if (currq->getVerticesCount() >= vcnt_required)
    {
        int edge_num = currq->getEdgesCount();
        std::vector<int> edge_invalid(edge_num, 0); //not 1:valid 1:invalid 2:can not be removed
        double ave_degree = currq->getAverageDegree();
        int vertex_num = currq->getVerticesCount();
        int real_edge_num = edge_num;
        if (isSparse)
        {
            //sparse: require degree<=degree_threshold
            if (ave_degree > degree_threshold)
            {
#if WRITE_TO_FILE_DEBUG == 1
                //删除边之前的图写文件
                std::string qfilename = "debug/query_";
                if (currq->isSparse)
                {
                    qfilename += "sparse_";
                }
                else
                {
                    qfilename += "dense_";
                }
                qfilename += std::to_string(vcnt_required) + "_";
                qfilename += std::to_string(curr_qcnt) + ".graph";
#if STEP_DEBUG == 1
                std::cout << "start write q to file: " << qfilename << std::endl;
#endif //STEP_DEBUG
                std::ofstream fout(qfilename);
                fout << "t " << vertex_num << " " << real_edge_num << "\n";
                std::vector<unsigned> vertices_sorted;
                currq->copyVerticesToVector(vertices_sorted);
                std::sort(vertices_sorted.begin(), vertices_sorted.end());
                std::map<unsigned, unsigned> newid;
                for (unsigned vid = 0; vid < vertex_num; ++vid)
                {
                    LabelID oldId = vertices_sorted[vid];
                    newid[oldId] = vid;
                    fout << "v " << vid << " " << dg->getVertexLabel(oldId) << " " << currq->getVertexDegree(oldId) << "\n";
                }
                unsigned fromId, toId;
                for (unsigned i = 0; i < edge_num; ++i)
                {
                    if (edge_invalid[i] != 1)
                    {
                        fromId = newid[currq->edge_from[i]];
                        toId = newid[currq->edge_to[i]];
                        fout << "e " << fromId << " " << toId << "\n";
                    }
                }
                fout.close();
#endif //WRITE_TO_FILE_DEBUG

                //remove edges at random
                int exceed_degree = currq->getTotalDegree() - degree_threshold * vertex_num;
                //remove 1 edge: total_degree-2
                int remove_edge_num = exceed_degree / 2;
                if (exceed_degree % 2)
                {
                    remove_edge_num++;
                }
                real_edge_num -= remove_edge_num;

                //这个注释掉了：avoid removing edges that leads to zero_degree vertex
                //avoid removing tree edges(这样可能破坏生成的图的弱联通性)
                int remove_idx;
                for (int i = 0; i < remove_edge_num;)
                {
                    for (remove_idx = Rand(edge_num); edge_invalid[remove_idx] != 0; remove_idx = Rand(edge_num))
                        ;
                    //try remove remove_idx edge
                    edge_invalid[remove_idx] = 1;      //marked as invalid
                    if (currq->isTreeEdge(remove_idx)) //tree edge
                    {
                        edge_invalid[remove_idx] = 2; //marked as cannot be removed
                    }
                    else
                    {
                        currq->remove_1edge(remove_idx);
                        ++i;
                    }
                    /*
                    else
                    {
                        if (currq->try_remove_1edge_update_degree(remove_idx))
                        {
                            ++i;
                        }
                        else
                        {
                            edge_invalid[remove_idx] = 2; //marked as cannot be removed
                        }
                    }
                    */
                }
            }
        }
        else
        {
            //dense: require degree>degree_threshold
            if (ave_degree <= degree_threshold)
            {
                //discard it
                return 0;
            }
        }

        /*
        write file
        */
        //query_dense_vcnt_currqcnt.graph
        std::string qfilename = dest_dir + "query_";
        if (currq->isSparse)
        {
            qfilename += "sparse_";
        }
        else
        {
            qfilename += "dense_";
        }
        qfilename += std::to_string(vcnt_required) + "_";
        qfilename += std::to_string(curr_qcnt) + ".graph";
#if STEP_DEBUG == 1
        std::cout << "start write q to file: " << qfilename << std::endl;
#endif //STEP_DEBUG
        std::ofstream fout(qfilename);
        fout << "t " << vertex_num << " " << real_edge_num << "\n";
        std::vector<unsigned> vertices_sorted;
        currq->copyVerticesToVector(vertices_sorted);
        std::sort(vertices_sorted.begin(), vertices_sorted.end());
        std::map<unsigned, unsigned> newid;
#if COLLECT_Q_FEATURE == 1
        std::string code;
#endif //COLLECT_Q_FEATURE
        for (unsigned vid = 0; vid < vertex_num; ++vid)
        {
            LabelID oldId = vertices_sorted[vid];
            newid[oldId] = vid;
#if WRITE_TO_FILE_DEBUG == 1
            fout << "v newid:" << vid << " oldid(+1):" << oldId + 1 << " " << char(dg->getVertexLabel(oldId) + 'A') << " " << currq->getVertexDegree(oldId) << "\n";
#else
            fout << "v " << vid << " " << dg->getVertexLabel(oldId) << " " << currq->getVertexDegree(oldId) << "\n";
#endif //WRITE_TO_FILE_DEBUG
#if COLLECT_Q_FEATURE == 1
            data_vertices_in_query_graph.insert(oldId);
            code += std::to_string(oldId) + "_";
#endif //COLLECT_Q_FEATURE
        }
#if COLLECT_Q_FEATURE == 1
        query_graph_kind.insert(code);
#endif //#if COLLECT_Q_FEATURE==1
        unsigned fromId, toId;
        for (unsigned i = 0; i < edge_num; ++i)
        {
            if (edge_invalid[i] != 1)
            {
                fromId = newid[currq->edge_from[i]];
                toId = newid[currq->edge_to[i]];
#if WRITE_TO_FILE_DEBUG == 1
                fout << "e oldid(+1):" << currq->edge_from[i] + 1 << char(dg->getVertexLabel(currq->edge_from[i]) + 'A') << " " << currq->edge_to[i] + 1 << char(dg->getVertexLabel(currq->edge_to[i]) + 'A') << "\n";
                fout << "  newid(  ):" << fromId << "  " << toId << "\n";
#else
                fout << "e " << fromId << " " << toId << "\n";
#endif //WRITE_TO_FILE_DEBUG
            }
        }
        fout.close();
        return 1;
    }

    currq->addVertex(start, dg);
#if STEP_DEBUG == 1
    std::cout << "after add vertex v" << start << ", curr_vcnt:" << currq->getVerticesCount() << std::endl;
#endif //STEP_DEBUG
    unsigned out_neighbors_cnt, in_neighbors_cnt;
    //ATTENTION: bi neighbors would occur in both out and in!!!
    const unsigned *out_neighbors = dg->getVertexOutNeighbors(start, out_neighbors_cnt);
    const unsigned *in_neighbors = dg->getVertexInNeighbors(start, in_neighbors_cnt);
    //if no neighbors:
    if (out_neighbors_cnt == 0 && in_neighbors_cnt == 0)
    {
        return 0;
    }
    //if all neigbors in currq:
    if (currq->allVerticesAlreadyIn(out_neighbors, out_neighbors_cnt) && currq->allVerticesAlreadyIn(in_neighbors, in_neighbors_cnt))
    {
        return 0;
    }

    bool is_out;
    unsigned neighbor = getRandFrom2Array(out_neighbors, in_neighbors, out_neighbors_cnt, in_neighbors_cnt, is_out);
    //choose next vertex at random (avoid already in currq)
    while (currq->isVertex(neighbor))
    {
        neighbor = getRandFrom2Array(out_neighbors, in_neighbors, out_neighbors_cnt, in_neighbors_cnt, is_out);
    }
    if (currq->getVerticesCount() <= vcnt_required)
    {
        if (is_out)
        {
            currq->insertTreeEdge(start, neighbor);
        }
        else
        {
            currq->insertTreeEdge(neighbor, start);
        }
    }
    return gen_qgraph_from_start(neighbor, vcnt_required, curr_qcnt, degree_threshold, dest_dir, currq, dg);
}

/* WARNNING:下面这个函数还没有修改达到顶点数目如果是sparse要随机去边的话，不能去掉树边
从out邻居中随机选取下一个顶点（则这样生成的查询图一定是强联通的） */
bool gen_qgraph_from_start_out(unsigned start, unsigned vcnt_required, unsigned curr_qcnt, unsigned degree_threshold, std::string dest_dir, QGraph *currq, Graph *dg)
{
    bool isSparse = currq->isSparse;
#if STEP_DEBUG == 1
    std::cout << "v" << start + 1 << " ";
    //std::cout << "visited:v" << start + 1 << " curr_vcnt:" << currq->getVerticesCount() << " vcnt_required:" << vcnt_required << std::endl;
#endif //STEP_DEBUG
    if (currq->getVerticesCount() >= vcnt_required)
    {
        int edge_num = currq->getEdgesCount();
        std::vector<int> edge_invalid(edge_num, 0); //not 1:valid 1:invalid 2:can not be removed
        double ave_degree = currq->getAverageDegree();
        int vertex_num = currq->getVerticesCount();
        int real_edge_num = edge_num;
        if (isSparse)
        {
            //sparse: require degree<=degree_threshold
            if (ave_degree > degree_threshold)
            {
                //remove edges at random
                int exceed_degree = currq->getTotalDegree() - degree_threshold * vertex_num;
                //remove 1 edge: total_degree-2
                int remove_edge_num = exceed_degree / 2;
                if (exceed_degree % 2)
                {
                    remove_edge_num++;
                }
                real_edge_num -= remove_edge_num;

                //avoid removing edges that leads to zero_degree vertex
                int remove_idx;
                for (int i = 0; i < remove_edge_num;)
                {
                    for (remove_idx = Rand(edge_num); edge_invalid[remove_idx] != 0; remove_idx = Rand(edge_num))
                        ;
                    //try remove remove_idx edge
                    edge_invalid[remove_idx] = 1; //marked as invalid
                    if (currq->try_remove_1edge_update_degree(remove_idx))
                    {
                        ++i;
                    }
                    else
                    {
                        edge_invalid[remove_idx] = 2; //marked as cannot be removed
                    }
                }
            }
        }
        else
        {
            //dense: require degree>degree_threshold
            if (ave_degree <= degree_threshold)
            {
                //discard it
                return 0;
            }
        }

        /*
        write file
        */
        //query_dense_vcnt_currqcnt.graph
        std::string qfilename = dest_dir + "query_";
        if (currq->isSparse)
        {
            qfilename += "sparse_";
        }
        else
        {
            qfilename += "dense_";
        }
        qfilename += std::to_string(vcnt_required) + "_";
        qfilename += std::to_string(curr_qcnt) + ".graph";
#if STEP_DEBUG == 1
        std::cout << "start write q to file: " << qfilename << std::endl;
#endif //STEP_DEBUG
        std::ofstream fout(qfilename);
        fout << "t " << vertex_num << " " << real_edge_num << "\n";
        std::vector<unsigned> vertices_sorted;
        currq->copyVerticesToVector(vertices_sorted);
        std::sort(vertices_sorted.begin(), vertices_sorted.end());
        std::map<unsigned, unsigned> newid;
#if COLLECT_Q_FEATURE == 1
        std::string code;
#endif //COLLECT_Q_FEATURE
        for (unsigned vid = 0; vid < vertex_num; ++vid)
        {
            LabelID oldId = vertices_sorted[vid];
            newid[oldId] = vid;
#if WRITE_TO_FILE_DEBUG == 1
            fout << "v newid:" << vid << " oldid(+1):" << oldId + 1 << " " << char(dg->getVertexLabel(oldId) + 'A') << " " << currq->getVertexDegree(oldId) << "\n";
#else
            fout << "v " << vid << " " << dg->getVertexLabel(oldId) << " " << currq->getVertexDegree(oldId) << "\n";
#endif //WRITE_TO_FILE_DEBUG
#if COLLECT_Q_FEATURE == 1
            data_vertices_in_query_graph.insert(oldId);
            code += std::to_string(oldId) + "_";
#endif //COLLECT_Q_FEATURE
        }
#if COLLECT_Q_FEATURE == 1
        query_graph_kind.insert(code);
#endif //#if COLLECT_Q_FEATURE==1
        unsigned fromId, toId;
        for (unsigned i = 0; i < edge_num; ++i)
        {
            if (edge_invalid[i] != 1)
            {
                fromId = newid[currq->edge_from[i]];
                toId = newid[currq->edge_to[i]];
#if WRITE_TO_FILE_DEBUG == 1
                fout << "e oldid(+1):" << currq->edge_from[i] + 1 << char(dg->getVertexLabel(currq->edge_from[i]) + 'A') << " " << currq->edge_to[i] + 1 << char(dg->getVertexLabel(currq->edge_to[i]) + 'A') << "\n";
                fout << "  newid(  ):" << fromId << "  " << toId << "\n";
#else
                fout << "e " << fromId << " " << toId << "\n";
#endif //WRITE_TO_FILE_DEBUG
            }
        }
        fout.close();
        return 1;
    }

    currq->addVertex(start, dg);
#if STEP_DEBUG == 1
    std::cout << "after add vertex v" << start << ", curr_vcnt:" << currq->getVerticesCount() << std::endl;
#endif //STEP_DEBUG
    unsigned out_neighbors_cnt;
    const unsigned *out_neighbors = dg->getVertexOutNeighbors(start, out_neighbors_cnt);
    //if no out neighbors:
    if (out_neighbors_cnt == 0)
    {
        return 0;
    }
    //if all neigbors in currq:
    if (currq->allVerticesAlreadyIn(out_neighbors, out_neighbors_cnt))
    {
        return 0;
    }

    unsigned neighbor = getRandFromArray(out_neighbors, out_neighbors_cnt);
    //choose next vertex at random (avoid already in currq)
    while (currq->isVertex(neighbor))
    {
        neighbor = getRandFromArray(out_neighbors, out_neighbors_cnt);
    }
    return gen_qgraph_from_start_out(neighbor, vcnt_required, curr_qcnt, degree_threshold, dest_dir, currq, dg);
}

/* WARNNING:硬编码call gen_qgraph_from_start
 * sparse_arr: {0} {1} {0,1}
 * dest_dir: write to dest_dir/query_dense_vcnt_currqcnt.graph  xxx/youtube/query_graph/
 * file format: t v e
 * when generating dense query_graph, it's possible to abort (when the function finds the data_graph is too sparse)
 */
void gen_qgraph(Graph *dg, std::vector<unsigned> vcnt_arr, std::vector<bool> sparse_arr, unsigned qcnt_required, std::string dest_dir, int GAP)
{
    unsigned dg_vertice_cnt = dg->getVerticesCount();
    for (bool isSparse : sparse_arr)
    {
        for (unsigned vcnt_required : vcnt_arr)
        {
            //qset: isSparse+vcnt_required
#if COLLECT_Q_FEATURE == 1 //collect statistics for this qset
            data_vertices_in_query_graph.clear();
            query_graph_kind.clear();
            //xxx/youtube/query_graph/
            unsigned tmpidx = dest_dir.find_last_of("/", dest_dir.size() - 2);
            std::string dest_dir_par = dest_dir.substr(0, tmpidx + 1);
            std::ofstream fout_q_properties(dest_dir_par + "Qfeatures.txt", std::ios::app);
            fout_q_properties << vcnt_required << " ";
            fout_q_properties << ((isSparse) ? "S" : "D") << std::endl;
#endif //COLLECT_Q_FEATURE

            unsigned curr_qcnt = 1;            //next write filename
            std::set<unsigned> start_vertices; //already start from them
            unsigned degree_threshold = 3;
            unsigned vcnt_ok_degree_to_sparse_cnt = 0;
            while (curr_qcnt <= qcnt_required)
            {
                if (start_vertices.size() >= dg_vertice_cnt)
                {
                    start_vertices.clear(); //another start turn
                }
                unsigned start = Rand(dg_vertice_cnt);
                while (start_vertices.count(start))
                {
                    start = Rand(dg_vertice_cnt);
                }
                start_vertices.insert(start);
                QGraph *currq = new QGraph(isSparse);
#if STEP_DEBUG == 1
                std::cout << "\ngenerate q from start vertex: v" << start + 1 << std::endl;
#endif //STEP_DEBUG
                if (gen_qgraph_from_start(start, vcnt_required, curr_qcnt, degree_threshold, dest_dir, currq, dg))
                {
                    ++curr_qcnt;
                    vcnt_ok_degree_to_sparse_cnt = 0;
                }
                else
                {
#if STEP_DEBUG == 1
                    std::cout << "no q found from start vertex" << std::endl;
#endif //STEP_DEBUG
                    if (currq->getVerticesCount() >= vcnt_required)
                    {
                        vcnt_ok_degree_to_sparse_cnt++;
                        if (vcnt_ok_degree_to_sparse_cnt > GAP)
                        {
                            std::cout
                                << "ERROR: data_graph too SPRASE:\n\tfor dest_dir:" << dest_dir << "\n\tqset:(vcnt:" << vcnt_required << ",DENSE)\n\tdegree_threshold:" << degree_threshold << "\n\tAbort." << std::endl;
                            ;
                            delete currq;
                            return; //all dense(means, vcnt_required) after it are omited (see outer loop: sparse_arr)
                        }
                    }
                }

                delete currq;
            }
#if COLLECT_Q_FEATURE == 1
            unsigned dg_vertex_num = dg->getVerticesCount();
            unsigned maxid = 0, minid = dg_vertex_num;
            double id_ave = 0, id_std = 0;
            unsigned cnt = 0;
            for (unsigned id : data_vertices_in_query_graph)
            {
                moving_average(id_ave, id, cnt);
                if (id > maxid)
                {
                    maxid = id;
                }
                if (id < minid)
                {
                    minid = id;
                }
            }
            cnt = 0;
            for (unsigned id : data_vertices_in_query_graph)
            {
                double new_val_for_std = (id - id_ave) * (id - id_ave);
                moving_average(id_std, new_val_for_std, cnt);
            }
            id_std = sqrt(id_std);
            fout_q_properties << data_vertices_in_query_graph.size() << "\n"
                              << maxid << " " << minid << "\n"
                              << id_ave << " " << id_std << "\n"
                              << dg_vertex_num << "\n"
                              << (data_vertices_in_query_graph.size() / (double)dg_vertex_num) << "\n"
                              << std::endl;
            fout_q_properties << query_graph_kind.size() << "\n"
                              << qcnt_required << "\n"
                              << std::endl;
            /*
            fout_q_properties << "different data_vertex in generated query_graph:" << data_vertices_in_query_graph.size() << "\nmaxid:" << maxid << " minid:" << minid << "\nid_ave:" << id_ave << " id_std:" << id_std << "\nall vertices:" << dg_vertex_num << "\nquery graph/all vertices: " << (data_vertices_in_query_graph.size() / (double)dg_vertex_num) << "\n"
                              << std::endl;
            fout_q_properties << "query_graph_kind with respect to vertices series(eg: q1 with v1v3v4 in g, 'is' the same as q2 with v1v3v4 in g): " << query_graph_kind.size() << "\nqcnt_required:" << qcnt_required << "\n"
                              << std::endl;
                              */
            fout_q_properties.close();
#endif //COLLECT_Q_FEATURE
        }
    }
}

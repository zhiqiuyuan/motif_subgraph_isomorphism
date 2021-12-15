#include "../inc/Database_process.h"
//#define GAP 100
int GAP;

#ifdef ONLINE_DEBUG
std::set<ui> data_vertices_in_query_graph;
std::set<std::string> query_graph_kind; //vertexId in query_graph coded into string; want to see how many kind
#endif                                  //ONLINE_DEBUG

//return [0,possible_max)
long long Rand(long long possible_max)
{
    //segment division
    double min_interval_width = possible_max / (double)RAND_MAX;
    long long re = rand();
    while (min_interval_width >= RAND_MAX)
    {
        re = re * RAND_MAX + rand();
        min_interval_width /= RAND_MAX;
    }
    return re % possible_max;
}

//concate as left+right: pick element from left+right at random
ui getRandFrom2Array(const ui *left, const ui *right, ui left_sz, ui right_sz)
{
    ui idx = Rand(left_sz + right_sz);
    if (idx < left_sz)
    {
        return left[idx];
    }
    return right[idx - left_sz];
}

//cnt: num of values included in old_ave
//???cnt???0?new_val???????old_ave ???????0?
void moving_average(double &old_ave, double new_val, ui &cnt)
{
    ++cnt;
    double tmp1 = old_ave * ((double(cnt - 1)) / cnt);
    double tmp2 = new_val / cnt;
    old_ave = tmp1 + tmp2;
}

//dfs from start
//curr_qcnt:for file name
//dest_dir: write to dest_dir/query_dense_vcnt_currqcnt.graph
//return whether found one
bool gen_qgraph_from_start(ui start, ui vcnt_required, ui curr_qcnt, ui degree_threshold, std::string dest_dir, QGraph *currq, Graph *dg)
{
    bool isSparse = currq->isSparse;
#ifdef DEBUG
    std::cout << "v" << start + 1 << " ";
    //std::cout << "visited:v" << start + 1 << " curr_vcnt:" << currq->getVerticesCount() << " vcnt_required:" << vcnt_required << std::endl;
#endif //DEBUG
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
#ifdef DEBUG
        std::cout << "start write q to file: " << qfilename << std::endl;
#endif //DEBUG
        std::ofstream fout(qfilename);
        fout << "t " << vertex_num << " " << real_edge_num << "\n";
        std::vector<ui> vertices_sorted;
        currq->copyVerticesToVector(vertices_sorted);
        std::sort(vertices_sorted.begin(), vertices_sorted.end());
        std::map<ui, ui> newid;
#ifdef ONLINE_DEBUG
        std::string code;
#endif //ONLINE_DEBUG
        for (ui vid = 0; vid < vertex_num; ++vid)
        {
            LabelID oldId = vertices_sorted[vid];
            newid[oldId] = vid;
#ifdef DEBUG
            fout << "v newid:" << vid << " oldid(+1):" << oldId + 1 << " " << char(dg->getVertexLabel(oldId) + 'A') << " " << currq->getVertexDegree(oldId) << "\n";
#else
            fout << "v " << vid << " " << dg->getVertexLabel(oldId) << " " << currq->getVertexDegree(oldId) << "\n";
#endif //DEBUG
#ifdef ONLINE_DEBUG
            data_vertices_in_query_graph.insert(oldId);
            code += std::to_string(oldId) + "_";
#endif //ONLINE_DEBUG
        }
#ifdef ONLINE_DEBUG
        query_graph_kind.insert(code);
#endif //#ifdef ONLINE_DEBUG
        ui fromId, toId;
        for (ui i = 0; i < edge_num; ++i)
        {
            if (edge_invalid[i] != 1)
            {
                fromId = newid[currq->edge_from[i]];
                toId = newid[currq->edge_to[i]];
#ifdef DEBUG
                fout << "e oldid(+1):" << currq->edge_from[i] + 1 << char(dg->getVertexLabel(currq->edge_from[i]) + 'A') << " " << currq->edge_to[i] + 1 << char(dg->getVertexLabel(currq->edge_to[i]) + 'A') << "\n";
                fout << "  newid(  ):" << fromId << "  " << toId << "\n";
#else
                fout << "e " << fromId << " " << toId << "\n";
#endif //DEBUG
            }
        }
        fout.close();
        return 1;
    }

    currq->addVertex(start, dg);
#ifdef DEBUG
    //std::cout << "after add vertex v" << start << ", curr_vcnt:" << currq->getVerticesCount() << std::endl;
#endif //DEBUG
    ui out_neighbors_cnt, in_neighbors_cnt;
    //ATTENTION: bi neighbors would occur in both out and in!!!
    const ui *out_neighbors = dg->getVertexOutNeighbors(start, out_neighbors_cnt);
    const ui *in_neighbors = dg->getVertexInNeighbors(start, in_neighbors_cnt);
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

    ui neighbor = getRandFrom2Array(out_neighbors, in_neighbors, out_neighbors_cnt, in_neighbors_cnt);
    //choose next vertex at random (avoid already in currq)
    while (currq->isVertex(neighbor))
    {
        neighbor = getRandFrom2Array(out_neighbors, in_neighbors, out_neighbors_cnt, in_neighbors_cnt);
    }
    return gen_qgraph_from_start(neighbor, vcnt_required, curr_qcnt, degree_threshold, dest_dir, currq, dg);
}

//sparse_arr: {0} {1} {0,1}
//dest_dir: write to dest_dir/query_dense_vcnt_currqcnt.graph  xxx/youtube/query_graph/
//when generating dense query_graph, it's possible to abort (when the function finds the data_graph is too sparse)
void gen_qgraph(Graph *dg, std::vector<ui> vcnt_arr, std::vector<bool> sparse_arr, ui qcnt_required, std::string dest_dir)
{
    ui dg_vertice_cnt = dg->getVerticesCount();
    for (bool isSparse : sparse_arr)
    {
        for (ui vcnt_required : vcnt_arr)
        {
            //qset: isSparse+vcnt_required
#ifdef ONLINE_DEBUG //collect statistics for this qset
            data_vertices_in_query_graph.clear();
            query_graph_kind.clear();
            ui tmpidx = dest_dir.find("query_graph");
            std::string dest_dir_par = dest_dir.substr(0, tmpidx);
            std::ofstream fout_q_properties(dest_dir_par + "properties_of_query_graph.txt", std::ios::app);
            fout_q_properties << vcnt_required << " ";
            fout_q_properties << ((isSparse) ? "S" : "D") << std::endl;
#endif //ONLINE_DEBUG

            ui curr_qcnt = 1;            //next write filename
            std::set<ui> start_vertices; //already start from them
            ui degree_threshold = 3;
            ui vcnt_ok_degree_to_sparse_cnt = 0;
            while (curr_qcnt <= qcnt_required)
            {
                if (start_vertices.size() >= dg_vertice_cnt)
                {
                    start_vertices.clear(); //another start turn
                }
                ui start = Rand(dg_vertice_cnt);
                while (start_vertices.count(start))
                {
                    start = Rand(dg_vertice_cnt);
                }
                start_vertices.insert(start);
                QGraph *currq = new QGraph(isSparse);
#ifdef DEBUG
                std::cout << "\ngenerate q from start vertex: v" << start + 1 << std::endl;
#endif //DEBUG
                if (gen_qgraph_from_start(start, vcnt_required, curr_qcnt, degree_threshold, dest_dir, currq, dg))
                {
                    ++curr_qcnt;
                    vcnt_ok_degree_to_sparse_cnt = 0;
                }
                else
                {
#ifdef DEBUG
                    std::cout << "no q found from start vertex" << std::endl;
#endif //DEBUG
                    if (currq->getVerticesCount() >= vcnt_required)
                    {
                        vcnt_ok_degree_to_sparse_cnt++;
                        if (vcnt_ok_degree_to_sparse_cnt > GAP)
                        {
                            std::string dgname;
                            //dest_dir: xxx/youtube/my_query_graph/
                            ui e = dest_dir.find_last_of('/');
                            e = dest_dir.find_last_of('/', e - 1);
                            ui b = dest_dir.find_last_of('/', e - 1);
                            dgname = dest_dir.substr(b + 1, e - b - 1);
                            std::cout
                                << "WARN: data_graph too SPRASE:\n\tfor data_graph:" << dgname << " qset:(vcnt:" << vcnt_required << ",DENSE)\n\tdegree_threshold:" << degree_threshold << "\n\tAbort." << std::endl;
                            ;
                            delete currq;
                            return; //all dense(means, vcnt_required) after it are omited (see outer loop: sparse_arr)
                        }
                    }
                }

                delete currq;
            }
#ifdef ONLINE_DEBUG
            ui dg_vertex_num = dg->getVerticesCount();
            ui maxid = 0, minid = dg_vertex_num;
            double id_ave = 0, id_std = 0;
            ui cnt = 0;
            for (ui id : data_vertices_in_query_graph)
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
            for (ui id : data_vertices_in_query_graph)
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
#endif //ONLINE_DEBUG
        }
    }
}

int main(int argc, char **argv)
{
    srand((unsigned)time(NULL));
    //-d dg_filename -v vcnt_arr -S sparse_arr -j qcnt_required
    std::string dg_filename; //"/media/data/hnu2022/yuanzhiqiu/word_test/data_graph/word_test.graph"; //xxx/data_graph/youtube.graph
    std::vector<ui> vcnt_arr = {8};
    std::vector<bool> sparse_arr = {0};
    ui qcnt_required = 200;
    if (argc < 9)
    {
        std::cout << "usage: ./dataset_pro -d dg_filename -v vcnt_arr -S sparse_arr -j qcnt_required" << std::endl;
        return 0;
    }
    int index = 1;
    while (index < argc)
    {
        std::string argstr = argv[index];
        if (argstr[0] == '-')
        {
            char c = argstr[1];
            if (c == 'd') //dg_filename
            {
                ++index;
                dg_filename = argv[index];
                ++index;
            }
            else if (c == 'v') //vcnt_arr
            {
                vcnt_arr.clear();
                ++index;
                argstr = argv[index];
                while (index < argc && argstr[0] != '-')
                {
                    vcnt_arr.push_back(stoi(argstr));
                    ++index;
                    if (index >= argc)
                    {
                        break;
                    }
                    argstr = argv[index];
                }
                //argstr[0] == '-': index is ok
                //index < argc: would break;
            }
            else if (c == 'S') //sparse_arr
            {
                sparse_arr.clear();
                ++index;
                argstr = argv[index];
                while (index < argc && argstr[0] != '-')
                {
                    sparse_arr.push_back(stoi(argstr));
                    ++index;
                    if (index >= argc)
                    {
                        break;
                    }
                    argstr = argv[index];
                }
                //argstr[0] == '-': index is ok
                //index < argc: would break;
            }
            else if (c == 'j') //qcnt_required
            {
                ++index;
                argstr = argv[index];
                qcnt_required = stoi(argstr);
                ++index;
            }
            else
            {
                std::cout << "not supported option: " << c << "\nusage: ./dataset_pro -d dg_filename -v vcnt_arr -S sparse_arr -j qcnt_required" << std::endl;
                return 0;
            }
        }
        else
        {
            std::cout << "error input. usage: ./dataset_pro -d dg_filename -v vcnt_arr -S sparse_arr -j qcnt_required" << std::endl;
        }
    }
    Graph *dg = new Graph();
    dg->loadGraphFromFile_directed(dg_filename);
    GAP = dg->getVerticesCount();
    GAP *= 2;
    ui idx = dg_filename.find("data_graph");
    std::string dest_dir_par = dg_filename.substr(0, idx);
    std::string dest_dir = dest_dir_par + "query_graph/";
#ifdef DEBUG
    //std::cout << "dest_dir: " << dest_dir << std::endl;
#endif //DEBUG
    gen_qgraph(dg, vcnt_arr, sparse_arr, qcnt_required, dest_dir);
    return 0;
}
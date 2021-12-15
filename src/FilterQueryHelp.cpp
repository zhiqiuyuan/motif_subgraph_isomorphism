#include "../inc/FilterQueryHelp.h"

double get_average(ui *candidates_count, ui vertex_num)
{
    if (vertex_num < 1)
    {
        return 0;
    }
    double ave = candidates_count[0];
    double cnt = 1;
    for (ui i = 1; i < vertex_num; ++i)
    {
        ++cnt;
        double tmp1 = ave * ((double(cnt - 1)) / cnt);
        double tmp2 = candidates_count[i] / cnt;
        ave = tmp1 + tmp2;
    }
    return ave;
}
double get_average(double *candidates_count, ui vertex_num)
{
    if (vertex_num < 1)
    {
        return 0;
    }
    double ave = candidates_count[0];
    double cnt = 1;
    for (ui i = 1; i < vertex_num; ++i)
    {
        ++cnt;
        double tmp1 = ave * ((double(cnt - 1)) / cnt);
        double tmp2 = candidates_count[i] / cnt;
        ave = tmp1 + tmp2;
    }
    return ave;
}

void moving_average(double &old_ave, double new_val, ui &cnt)
{
    ++cnt;
    double tmp1 = old_ave * ((double(cnt - 1)) / cnt);
    double tmp2 = new_val / cnt;
    old_ave = tmp1 + tmp2;
}
void moving_average(double *old_ave, double *new_val, ui &cnt, ui sz)
{
    ++cnt;
    for (ui i = 0; i < sz; ++i)
    {
        double tmp1 = old_ave[i] * ((double(cnt - 1)) / cnt);
        double tmp2 = new_val[i] / cnt;
        old_ave[i] = tmp1 + tmp2;
    }
}

void moving_average(kMethodCandiScale *old_ave, kMethodCandiScale *new_val, ui &cnt, ui sz)
{
    if (cnt == 0) //old_ave初始随意给，所以第一次求moving ave时要设置methodID和k
    {
        for (ui i = 0; i < sz; ++i)
        {
            old_ave[i].methodID = new_val[i].methodID;
            old_ave[i].k = new_val[i].k;
        }
    }
    ++cnt;
    for (ui i = 0; i < sz; ++i)
    {
        double tmp1 = old_ave[i].candiScale * ((double(cnt - 1)) / cnt);
        double tmp2 = new_val[i].candiScale / cnt;
        old_ave[i].candiScale = tmp1 + tmp2;
    }
}

#if DIRECTED_GRAPH == 1 && LABEL_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
void dLmtf_filterCandi_vary_k_method(std::string filename_prefix, kMethodCandiScale *&lmtf_candiScale_arr, ui kb, ui ke, ui kInFileName, std::string *method_set, ui KMETHOD_NUM, Graph *data_graph, Graph *query_graph)
{
    if (kb > ke)
    {
        std::cout << "kb<=ke is needed! kb:" << kb << " ke:" << ke << std::endl;
        return;
    }
    ui **candidates;
    ui *candidates_count;
    ui max_kind, min_kind;
    lmtf_candiScale_arr = new kMethodCandiScale[KMETHOD_NUM * (ke - kb + 1)];
    ui idx = 0;
    ui q_vertex_num = query_graph->getVerticesCount();
    for (ui i = 0; i < KMETHOD_NUM; ++i)
    { //method
        for (ui k = kb; k <= ke; ++k)
        {
            data_graph->loadMotifCountFromFile_label(filename_prefix, kInFileName, method_set[i], k);

            candidates = 0;
            candidates_count = 0;
            if (FilterVertices::LabelMotifFilter(data_graph, query_graph, candidates, candidates_count))
            {
                lmtf_candiScale_arr[idx].candiScale = get_average(candidates_count, q_vertex_num);
            }
            else
            {
                lmtf_candiScale_arr[idx].candiScale = 0;
            }
#ifdef DEBUG
            std::cout << method_set[i] << k << std::endl;
            print_candidates(candidates, candidates_count, q_vertex_num);
            std::cout << method_set[i] << k << " ave:" << lmtf_candiScale_arr[idx].candiScale << std::endl;
#endif //DEBUG
            lmtf_candiScale_arr[idx].k = k;
            lmtf_candiScale_arr[idx].methodID = i;
            ++idx;

            data_graph->delete_label_motif_count_();
            for (ui ii = 0; ii < q_vertex_num; ++ii)
            {
                delete[] candidates[ii];
            }
            delete[] candidates;
            delete[] candidates_count;
        }
    }
}
void print_scale_and_diff(double nlf_candiScale, std::string other_comment, kMethodCandiScale *lmtf_candiScale_arr, std::string lmtf_comment, std::string *method_set, ui sz)
{
    std::sort(lmtf_candiScale_arr, lmtf_candiScale_arr + sz); //candiScale ascending -> k ascending
    ui col_per_page = 6;
    std::cout << std::fixed << std::setprecision(2);

    for (ui i = 0; i < sz; ++i)
    {
        lmtf_candiScale_arr[i].rate = (nlf_candiScale - lmtf_candiScale_arr[i].candiScale) / nlf_candiScale * 100;
    }

#ifdef ONLINE_DEBUG //print all lmtf_candiScale_arr
    for (ui i = 0; i < sz; i += col_per_page)
    {
        print_scale_and_diff_inner(nlf_candiScale, other_comment, lmtf_candiScale_arr + i, lmtf_comment, method_set, std::min(col_per_page, sz - i));
        std::cout << std::endl;
    }
#endif //ONLINE_DEBUG

    //remove dup according to rate "Removes all but the first element from every consecutive group of equivalent elements"
    auto del = [](kMethodCandiScale i, kMethodCandiScale j) {
        return std::abs(i.rate - j.rate) < 0.005;
    };
    kMethodCandiScale *it = std::unique(lmtf_candiScale_arr, lmtf_candiScale_arr + sz, del);
    sz = it - lmtf_candiScale_arr;
    std::cout << "after remove dup, the size of lmtf_candiScale_arr is: " << sz << std::endl;
    for (ui i = 0; i < sz; i += col_per_page)
    {
        print_scale_and_diff_inner(nlf_candiScale, other_comment, lmtf_candiScale_arr + i, lmtf_comment, method_set, std::min(col_per_page, sz - i));
        std::cout << std::endl;
    }
}
void print_scale_and_diff_inner(double nlf_candiScale, std::string other_comment, kMethodCandiScale *lmtf_candiScale_arr, std::string lmtf_comment, std::string *method_set, ui sz)
{
    std::cout << std::fixed << std::setprecision(2);
    char split_c = '\t';
    //method
    std::cout << "method:" << split_c;
    for (ui i = 0; i < sz; ++i)
    {
        std::cout << method_set[lmtf_candiScale_arr[i].methodID] << split_c;
    }
    std::cout << std::endl;

    //k
    std::cout << "k:" << split_c;
    for (ui i = 0; i < sz; ++i)
    {
        std::cout << lmtf_candiScale_arr[i].k << split_c;
    }
    std::cout << std::endl;

    //lmtf candiScale
    std::cout << lmtf_comment << ":" << split_c;
    for (ui i = 0; i < sz; ++i)
    {
        std::cout << lmtf_candiScale_arr[i].candiScale << split_c;
    }
    std::cout << std::endl;

    //ohter candiScale
    std::cout << other_comment << ":" << split_c;
    for (ui i = 0; i < sz; ++i)
    {
        std::cout << nlf_candiScale << split_c;
    }
    std::cout << std::endl;

    //diff rate
    std::cout << "rate%:" << split_c;
    for (ui i = 0; i < sz; ++i)
    {
        std::cout << lmtf_candiScale_arr[i].rate << split_c;
    }
    std::cout << std::endl;
}

void bulkq_dLmtf_filterCandi_vary_k_method_v1(std::string filename_prefix, std::string qfilename_prefix, kMethodCandiScale *&lmtf_ave_candiScale_arr, ui kb, ui ke, ui kInFileName, std::string *method_set, ui KMETHOD_NUM, Graph *data_graph, ui qVScale, int jb, int je)
{
    ui idx = qfilename_prefix.find_last_of('/');
    std::string filename = qfilename_prefix.substr(idx + 1);
    qfilename_prefix = qfilename_prefix.substr(0, idx) + "/query_";
    if (filename == "SPARSE")
    {
        filename = qfilename_prefix + "sparse_";
    }
    else if (filename == "DENSE")
    {
        filename = qfilename_prefix + "dense_";
    }
    filename += std::to_string(qVScale) + "_";

    kMethodCandiScale *lmtf_candiScale_arr;
    ui kindnum = KMETHOD_NUM * (ke - kb + 1);
    lmtf_ave_candiScale_arr = new kMethodCandiScale[kindnum];
    ui max_kind, min_kind;
    ui cnt = 0;
    std::string filename_pre = filename;
    for (int j = jb; j <= je; ++j)
    {
        filename = filename_pre + std::to_string(j);
        std::cout << "dLmtf_filterCandi:[kb=" << kb << " ke=" << ke << "]:" << filename << std::endl;
        Graph *query_graph = new Graph();
        query_graph->loadGraphFromFile_directed(filename + ".graph");

        query_graph->generateMotifCount_label(filename);
        dLmtf_filterCandi_vary_k_method(filename_prefix, lmtf_candiScale_arr, kb, ke, kInFileName, method_set, KMETHOD_NUM, data_graph, query_graph);
        moving_average(lmtf_ave_candiScale_arr, lmtf_candiScale_arr, cnt, kindnum);
        delete[] lmtf_candiScale_arr;
        delete query_graph;
    }
}

//filename_prefix:"xxx/youtube"，会从xxx/youtube_directed_label_top_k.txt中加载G的motif_struct
//qfilename_prefix:"xxx/SPARSE"，会从xxx/query_sparse_vertexScale_j.graph加载q，会将结果写入xxx/query_sparse_vertexScale_j_directed（或undirected）.txt中
void bulkq_dLmtf_filterCandi_vary_k_method(std::string filename_prefix, std::string qfilename_prefix, kMethodCandiScale *&lmtf_ave_candiScale_arr, ui kb, ui ke, ui kInFileName, std::string *method_set, ui KMETHOD_NUM, Graph *data_graph, ui qVScale, int jb, int je, bool one_step)
{
    if (kb > ke)
    {
        std::cout << "kb<=ke is needed! kb:" << kb << " ke:" << ke << std::endl;
        return;
    }
    ui i = qfilename_prefix.find_last_of('/');
    std::string qfilename_pre = qfilename_prefix.substr(i + 1); //SPARSE
    qfilename_prefix = qfilename_prefix.substr(0, i) + "/query_";
    if (qfilename_pre == "SPARSE")
    {
        qfilename_pre = qfilename_prefix + "sparse_";
    }
    else if (qfilename_pre == "DENSE")
    {
        qfilename_pre = qfilename_prefix + "dense_";
    }
    qfilename_pre += std::to_string(qVScale) + "_"; //xxx/query_sparse_vertexScale_

    ui kindnum = KMETHOD_NUM * (ke - kb + 1);
    lmtf_ave_candiScale_arr = new kMethodCandiScale[kindnum];

    ui **candidates;
    ui *candidates_count;
    ui idx = 0;
    std::string qfilename;
    for (ui i = 0; i < KMETHOD_NUM; ++i)
    { //method
        //std::cout << "dLmtf_filterCandi:[kmethod=" << method_set[i] << "]:" << std::endl;
        for (ui k = kb; k <= ke; ++k)
        {
            data_graph->loadMotifCountFromFile_label(filename_prefix, kInFileName, method_set[i], k);
            std::cout << "[k=" << k << " kmethod=" << method_set[i] << "]:" << std::endl;
#ifdef DEBUG
            data_graph->print_graph_mtf(0);
#endif //#ifdef DEBUG
            lmtf_ave_candiScale_arr[idx].k = k;
            lmtf_ave_candiScale_arr[idx].methodID = i;
            ui cnt = 0;
            double candiScale = 0; //init value of "old value" in moving_average function doesn't matter
            for (int j = jb; j <= je; ++j)
            {
                candidates = 0;
                candidates_count = 0;

                qfilename = qfilename_pre + std::to_string(j);

                Graph *query_graph = new Graph();
                query_graph->loadGraphFromFile_directed(qfilename + ".graph");
                query_graph->generateMotifCount_label(qfilename);
                std::cout << "j:" << j << std::endl;
#ifdef DEBUG
                query_graph->print_graph_mtf(1);
#endif //DEBUG
                double new_candiScale;
                if (one_step == 0)
                {
                    if (FilterVertices::LabelMotifFilter(data_graph, query_graph, candidates, candidates_count))
                    {
                        new_candiScale = get_average(candidates_count, qVScale);
                    }
                    else
                    {
                        new_candiScale = 0;
                    }
                }
                else
                {
                    if (FilterVertices::LabelMotifFilter_1step(data_graph, query_graph, candidates, candidates_count))
                    {
                        new_candiScale = get_average(candidates_count, qVScale);
                    }
                    else
                    {
                        new_candiScale = 0;
                    }
                }
                moving_average(candiScale, new_candiScale, cnt);
#ifdef DEBUG
                std::cout << method_set[i] << k << std::endl;
                print_candidates(candidates, candidates_count, qVScale);
                std::cout << method_set[i] << k << " ave:" << new_candiScale << std::endl;
                std::cout << "after moving, new ave is:" << candiScale << std::endl;
#endif //DEBUG
                delete query_graph;
                for (ui ii = 0; ii < qVScale; ++ii)
                {
                    delete[] candidates[ii];
                }
                delete[] candidates;
                delete[] candidates_count;
            }
            lmtf_ave_candiScale_arr[idx].candiScale = candiScale;
            ++idx;
            data_graph->delete_label_motif_count_();
        }
    }
}

void bulkq_dLmtf_filterCandi_vary_k_method_collect_data_feature(std::string filename_prefix, std::string qfilename_prefix, kMethodCandiScale *&lmtf_ave_candiScale_arr, ui kb, ui ke, ui kInFileName, std::string *method_set, ui KMETHOD_NUM, Graph *data_graph, ui qVScale, int jb, int je, ui *feature)
{
    if (kb > ke)
    {
        std::cout << "kb<=ke is needed! kb:" << kb << " ke:" << ke << std::endl;
        return;
    }
    ui i = qfilename_prefix.find_last_of('/');
    std::string qfilename_pre = qfilename_prefix.substr(i + 1); //SPARSE
    qfilename_prefix = qfilename_prefix.substr(0, i) + "/query_";
    if (qfilename_pre == "SPARSE")
    {
        qfilename_pre = qfilename_prefix + "sparse_";
    }
    else if (qfilename_pre == "DENSE")
    {
        qfilename_pre = qfilename_prefix + "dense_";
    }
    qfilename_pre += std::to_string(qVScale) + "_"; //xxx/query_sparse_vertexScale_

    ui kindnum = KMETHOD_NUM * (ke - kb + 1);
    lmtf_ave_candiScale_arr = new kMethodCandiScale[kindnum];

    ui **candidates;
    ui *candidates_count;
    ui idx = 0;
    std::string qfilename;
    for (ui i = 0; i < KMETHOD_NUM; ++i)
    { //method
        //std::cout << "dLmtf_filterCandi:[kmethod=" << method_set[i] << "]:" << std::endl;
        for (ui k = kb; k <= ke; ++k)
        {
            data_graph->loadMotifCountFromFile_label(filename_prefix, kInFileName, method_set[i], k);
            std::cout << "[k=" << k << " kmethod=" << method_set[i] << "]:" << std::endl;
#ifdef DEBUG
            data_graph->print_graph_mtf(0);
#endif //#ifdef DEBUG
            lmtf_ave_candiScale_arr[idx].k = k;
            lmtf_ave_candiScale_arr[idx].methodID = i;
            ui cnt = 0;
            double candiScale = 0; //init value of "old value" in moving_average function doesn't matter
            for (int j = jb; j <= je; ++j)
            {
                candidates = 0;
                candidates_count = 0;

                qfilename = qfilename_pre + std::to_string(j);

                Graph *query_graph = new Graph();
                query_graph->loadGraphFromFile_directed(qfilename + ".graph");
                query_graph->generateMotifCount_label(qfilename);
                std::cout << "j:" << j << std::endl;
#ifdef DEBUG
                query_graph->print_graph_mtf(1);
#endif //DEBUG
                double new_candiScale;

                if (FilterVertices::LabelMotifFilter_collect_data_feature(data_graph, query_graph, candidates, candidates_count, feature))
                {
                    new_candiScale = get_average(candidates_count, qVScale);
                }
                else
                {
                    new_candiScale = 0;
                }

                moving_average(candiScale, new_candiScale, cnt);
#ifdef DEBUG
                std::cout << method_set[i] << k << std::endl;
                print_candidates(candidates, candidates_count, qVScale);
                std::cout << method_set[i] << k << " ave:" << new_candiScale << std::endl;
                std::cout << "after moving, new ave is:" << candiScale << std::endl;
#endif //DEBUG
                delete query_graph;
                for (ui ii = 0; ii < qVScale; ++ii)
                {
                    delete[] candidates[ii];
                }
                delete[] candidates;
                delete[] candidates_count;
            }
            lmtf_ave_candiScale_arr[idx].candiScale = candiScale;
            ++idx;
            data_graph->delete_label_motif_count_();
        }
    }
}

#if LABEL_MOTIF_LIMIT == 1
//qfilename_prefix:xxx/DENSE
void bulkq_lmtf_limit_filter(std::string qfilename_prefix, Graph *data_graph, ui qVScale, int jb, int je, double &candiScale_ave)
{
    ui idx = qfilename_prefix.find_last_of('/');
    std::string filename = qfilename_prefix.substr(idx + 1);        //DENSE
    qfilename_prefix = qfilename_prefix.substr(0, idx) + "/query_"; //xxx/query_
    if (filename == "SPARSE")
    {
        filename = qfilename_prefix + "sparse_"; //xxx/query_sparse
    }
    else if (filename == "DENSE")
    {
        filename = qfilename_prefix + "dense_";
    }
    filename += std::to_string(qVScale) + "_"; //xxx/query_sparse_qVScale_
    ui **candidates;
    ui *candidates_count;
    double candiScale;
    ui cnt = 0;
    std::string filename_prefix = filename; //xxx/query_sparse_qVScale_
    for (int j = jb; j <= je; ++j)
    {
        filename = filename_prefix + std::to_string(j) + ".graph";
        //std::cout << filename << std::endl;
        std::cout << "j:" << j << std::endl;
        Graph *query_graph = new Graph();
        query_graph->loadGraphFromFile_directed(filename);
        query_graph->generateMotifCount_label_limit(filename.substr(0, filename.find('.')));
        candidates = 0;
        candidates_count = 0;
        if (FilterVertices::LabelMotifFilter_limit(data_graph, query_graph, candidates, candidates_count))
        {
            candiScale = get_average(candidates_count, qVScale);
        }
        else
        {
            candiScale = 0;
        }

#ifdef DEBUG
        print_candidates(candidates, candidates_count, qVScale);
        std::cout << "this ave:" << candiScale << std::endl;
#endif //DEBUG
        moving_average(candiScale_ave, candiScale, cnt);
#ifdef DEBUG
        std::cout << "after moving, the new ave:" << candiScale_ave << std::endl;
#endif //DEBUG
        for (ui i = 0; i < qVScale; ++i)
        {
            delete[] candidates[i];
        }
        delete[] candidates;
        delete[] candidates_count;
        delete query_graph;
    }
}

#endif //LABEL_MOTIF_LIMIT

#endif // DIRECTED_GRAPH == 1 && LABEL_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1

//qfilename_prefix:"xxx/SPRASE"
void bulkq_nlf_filterCandi(std::string qfilename_prefix, double &nlf_ave_candiScale, Graph *data_graph, ui qVScale, int jb, int je, bool one_step)
{
    ui idx = qfilename_prefix.find_last_of('/');
    std::string filename = qfilename_prefix.substr(idx + 1);
    qfilename_prefix = qfilename_prefix.substr(0, idx) + "/query_";
    if (filename == "SPARSE")
    {
        filename = qfilename_prefix + "sparse_";
    }
    else if (filename == "DENSE")
    {
        filename = qfilename_prefix + "dense_";
    }
    filename += std::to_string(qVScale) + "_"; //xxx/query_sparse_qVScale_
    ui **candidates;
    ui *candidates_count;
    double nlf_candiScale;
    ui cnt = 0;
    std::string filename_prefix = filename;
    if (one_step == 0)
    {
        for (int j = jb; j <= je; ++j)
        {
            filename = filename_prefix + std::to_string(j) + ".graph";
            //std::cout << filename << std::endl;
            std::cout << "j:" << j << std::endl;
            Graph *query_graph = new Graph();
            query_graph->loadGraphFromFile_directed(filename);

            candidates = 0;
            candidates_count = 0;
            if (FilterVertices::NLFFilter(data_graph, query_graph, candidates, candidates_count))
            {
                nlf_candiScale = get_average(candidates_count, qVScale);
            }
            else
            {
                nlf_candiScale = 0;
            }

#ifdef DEBUG
            print_candidates(candidates, candidates_count, qVScale);
            std::cout << "this ave:" << nlf_candiScale << std::endl;
#endif //DEBUG
            moving_average(nlf_ave_candiScale, nlf_candiScale, cnt);
#ifdef DEBUG
            std::cout << "after moving, the new ave:" << nlf_ave_candiScale << std::endl;
#endif //DEBUG
            for (ui i = 0; i < qVScale; ++i)
            {
                delete[] candidates[i];
            }
            delete[] candidates;
            delete[] candidates_count;
            delete query_graph;
        }
    }
    else
    {
        for (int j = jb; j <= je; ++j)
        {
            filename = filename_prefix + std::to_string(j) + ".graph";
            //std::cout << filename << std::endl;
            std::cout << "j:" << j << std::endl;
            Graph *query_graph = new Graph();
            query_graph->loadGraphFromFile_directed(filename);
#ifdef ONLINE_DEBUG
            std::cout << "query_graph:" << std::endl;
            query_graph->print_graph_detail();
            std::cout << "data_graph:" << std::endl;
            data_graph->print_graph_detail();
#endif //ONLINE_DEBUG
            candidates = 0;
            candidates_count = 0;
            if (FilterVertices::NLFFilter_1step(data_graph, query_graph, candidates, candidates_count))
            {
                nlf_candiScale = get_average(candidates_count, qVScale);
            }
            else
            {
                nlf_candiScale = 0;
            }

#ifdef DEBUG
            print_candidates(candidates, candidates_count, qVScale);
            std::cout << "this ave:" << nlf_candiScale << std::endl;
#endif //DEBUG
            moving_average(nlf_ave_candiScale, nlf_candiScale, cnt);
#ifdef DEBUG
            std::cout << "after moving, the new ave:" << nlf_ave_candiScale << std::endl;
#endif //DEBUG
            for (ui i = 0; i < qVScale; ++i)
            {
                delete[] candidates[i];
            }
            delete[] candidates;
            delete[] candidates_count;
            delete query_graph;
        }
    }
}

//qfilename_prefix:"xxx/SPRASE"
void bulkq_ldf_filterCandi(std::string qfilename_prefix, double &nlf_ave_candiScale, Graph *data_graph, ui qVScale, int jb, int je)
{
    ui idx = qfilename_prefix.find_last_of('/');
    std::string filename = qfilename_prefix.substr(idx + 1);
    qfilename_prefix = qfilename_prefix.substr(0, idx) + "/query_";
    if (filename == "SPARSE")
    {
        filename = qfilename_prefix + "sparse_";
    }
    else if (filename == "DENSE")
    {
        filename = qfilename_prefix + "dense_";
    }
    filename += std::to_string(qVScale) + "_"; //xxx/query_sparse_qVScale_
    ui **candidates;
    ui *candidates_count;
    double nlf_candiScale;
    ui cnt = 0;
    std::string filename_prefix = filename;
    for (int j = jb; j <= je; ++j)
    {
        filename = filename_prefix + std::to_string(j) + ".graph";
        //std::cout << filename << std::endl;
        std::cout << "j:" << j << std::endl;
        Graph *query_graph = new Graph();
        query_graph->loadGraphFromFile_directed(filename);

        candidates = 0;
        candidates_count = 0;
        if (FilterVertices::LDFFilter(data_graph, query_graph, candidates, candidates_count))
        {
            nlf_candiScale = get_average(candidates_count, qVScale);
        }
        else
        {
            nlf_candiScale = 0;
        }

#ifdef DEBUG
        print_candidates(candidates, candidates_count, qVScale);
        std::cout << "this ave:" << nlf_candiScale << std::endl;
#endif //DEBUG
        moving_average(nlf_ave_candiScale, nlf_candiScale, cnt);
#ifdef DEBUG
        std::cout << "after moving, the new ave:" << nlf_ave_candiScale << std::endl;
#endif //DEBUG
        for (ui i = 0; i < qVScale; ++i)
        {
            delete[] candidates[i];
        }
        delete[] candidates;
        delete[] candidates_count;
        delete query_graph;
    }
}

#if (DIRECTED_GRAPH == 1 || LABEL_MOTIF_ENABLE == 1) && ONLINE_STAGE == 1
//k and motif structure already loaded in G
//qfilename:xxxx/query_sparse_qVScale_j.graph
void mtf_nlf_filter(std::string qfilename, Graph *data_graph, Graph *query_graph, bool label, double &mtf_candiScale, double &nlf_candiScale)
{
    ui idx = qfilename.find_last_of('.');
    qfilename = qfilename.substr(0, idx); //xxxx/query_sparse_qVScale_j
    ui **candidates;
    ui *candidates_count;
    ui q_vertex_num = query_graph->getVerticesCount();
#if TOPO_MOTIF_ENABLE == 1 && LABEL_MOTIF_ENABLE == 1
    if (label == 0)
    {
        query_graph->generateMotifCount(qfilename);
        FilterVertices::MotifFilter(data_graph, query_graph, candidates, candidates_count, 1);
    }
    else
    {
        query_graph->generateMotifCount_label(qfilename);
        FilterVertices::LabelMotifFilter(data_graph, query_graph, candidates, candidates_count);
    }
#endif //#if TOPO_MOTIF_ENABLE==1 && LABEL_MOTIF_ENABLE==1
#if TOPO_MOTIF_ENABLE == 1 && LABEL_MOTIF_ENABLE == 0
    query_graph->generateMotifCount(qfilename);
    /*
    FilterVertices::MotifFilter(data_graph, query_graph, candidates, candidates_count);
    */
    FilterVertices::MotifFilter_1step(data_graph, query_graph, candidates, candidates_count, 1);
#endif //#if TOPO_MOTIF_ENABLE==1 && LABEL_MOTIF_ENABLE==0
#if TOPO_MOTIF_ENABLE == 0 && LABEL_MOTIF_ENABLE == 1
    query_graph->generateMotifCount_label(qfilename);
    FilterVertices::LabelMotifFilter(data_graph, query_graph, candidates, candidates_count);
#endif //#if TOPO_MOTIF_ENABLE==0 && LABEL_MOTIF_ENABLE==1
    mtf_candiScale = get_average(candidates_count, q_vertex_num);
    for (ui i = 0; i < q_vertex_num; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;

    candidates = NULL;
    candidates_count = NULL;
    FilterVertices::NLFFilter(data_graph, query_graph, candidates, candidates_count);
    nlf_candiScale = get_average(candidates_count, q_vertex_num);

    for (ui i = 0; i < q_vertex_num; ++i)
    {
        delete[] candidates[i];
    }
    delete[] candidates;
    delete[] candidates_count;
}

/*
（所有数据库的结果写到一个文件/media/data/hnu2022/yuanzhiqiu/filter_candiScale/lmtf_CandiScale.txt里面）
[candiScale ascending -> k ascending]
youtube
qVScale1 S/D
kmethod1 k1 candiScale1 
kmethod2 k2 candiScale2 
...
youtube
qVScale2 S/D
...
hprd
...
*/
//input_data_graph_file_prefix:/media/data/hnu2022/yuanzhiqiu/youtube/data_graph /youtube
void write_lmtf_CandiScale(kMethodCandiScale *lmtf_ave_candiScale_arr, std::string *method_set, ui sz, ui q_vertexScale, std::string sparse_str, std::string result_fname_prefix, std::string dg_name, std::string filterTypeName, double duration)
{
    std::ofstream fout(result_fname_prefix + filterTypeName + "_CandiScale.txt", std::ios::app);
    std::string split_c = " ";
    fout << dg_name << "\n";
    fout << q_vertexScale << split_c << sparse_str[0] << "\n";

    //candiscale ascending -> k ascending
    std::sort(lmtf_ave_candiScale_arr, lmtf_ave_candiScale_arr + sz);
    //remove dup according to candiscale
    //remove dup according to rate "Removes all but the first element from every consecutive group of equivalent elements"
    auto del = [](kMethodCandiScale i, kMethodCandiScale j) {
        return std::abs(i.candiScale - j.candiScale) < 0.05;
    };
    kMethodCandiScale *it = std::unique(lmtf_ave_candiScale_arr, lmtf_ave_candiScale_arr + sz, del);
    sz = it - lmtf_ave_candiScale_arr;

    for (ui i = 0; i < sz; ++i)
    {
        fout << method_set[lmtf_ave_candiScale_arr[i].methodID] << split_c << lmtf_ave_candiScale_arr[i].k << split_c << lmtf_ave_candiScale_arr[i].candiScale << "\n";
    }
    fout << duration << "\n\n";

    fout.close();
}

//qfilename_prefix:xxx/SPARSE
void bulkq_mtf_nlf_filter(std::string qfilename_prefix, Graph *data_graph, ui qVScale, int jb, int je, bool label, double &mtf_candiScale_ave, double &nlf_candiScale_ave)
{
    ui idx = qfilename_prefix.find_last_of('/');
    std::string filename = qfilename_prefix.substr(idx + 1);        //SPARSE
    qfilename_prefix = qfilename_prefix.substr(0, idx) + "/query_"; //xxxx/query_
    if (filename == "SPARSE")
    {
        filename = qfilename_prefix + "sparse_";
    }
    else if (filename == "DENSE")
    {
        filename = qfilename_prefix + "dense_";
    }
    filename += std::to_string(qVScale) + "_"; //xxxx/query_sparse_qVScale_

    ui **candidates;
    ui *candidates_count;
    ui jsize = je - jb + 1;
    double *mtf_candiScale_arr = new double[jsize];
    double *nlf_candiScale_arr = new double[jsize];

    ui cnt = 0;
    std::string filename_pre = filename; //xxxx/query_sparse_qVScale_
    for (int j = jb; j <= je; ++j)
    {
        filename = filename_pre + std::to_string(j) + ".graph";
        Graph *query_graph = new Graph();
#if DIRECTED_GRAPH == 1
        query_graph->loadGraphFromFile_directed(filename);
#else
        query_graph->loadGraphFromFile(filename);
#endif //DIRECTED_GRAPH == 1

        mtf_nlf_filter(filename, data_graph, query_graph, label, mtf_candiScale_arr[j - jb], nlf_candiScale_arr[j - jb]);
        delete query_graph;
    }

    mtf_candiScale_ave = get_average(mtf_candiScale_arr, jsize);
    nlf_candiScale_ave = get_average(nlf_candiScale_arr, jsize);

    delete[] mtf_candiScale_arr;
    delete[] nlf_candiScale_arr;
}

#if TOPO_MOTIF_ENABLE == 1
void bulkq_mtf_filter(std::string qfilename_prefix, Graph *data_graph, ui qVScale, int jb, int je, double &mtf_candiScale_ave, bool do_one_step, bool use_nlf)
{
    ui idx = qfilename_prefix.find_last_of('/');
    std::string filename = qfilename_prefix.substr(idx + 1);        //SPARSE
    qfilename_prefix = qfilename_prefix.substr(0, idx) + "/query_"; //xxxx/query_
    if (filename == "SPARSE")
    {
        filename = qfilename_prefix + "sparse_";
    }
    else if (filename == "DENSE")
    {
        filename = qfilename_prefix + "dense_";
    }
    filename += std::to_string(qVScale) + "_"; //xxxx/query_sparse_qVScale_

    ui **candidates;
    ui *candidates_count;
    ui jsize = je - jb + 1;
    double mtf_candiScale;

    ui cnt = 0;
    std::string filename_pre = filename; //xxxx/query_sparse_qVScale_
    for (int j = jb; j <= je; ++j)
    {
        filename = filename_pre + std::to_string(j) + ".graph";
        Graph *query_graph = new Graph();
#if DIRECTED_GRAPH == 1
        query_graph->loadGraphFromFile_directed(filename);
#else
        query_graph->loadGraphFromFile(filename);
#endif //DIRECTED_GRAPH == 1

        std::cout << "j:" << j << std::endl;
        query_graph->generateMotifCount(filename.substr(0, filename.find_last_of('.')));

        candidates = 0;
        candidates_count = 0;
        if (do_one_step == 0)
        {
            if (FilterVertices::MotifFilter(data_graph, query_graph, candidates, candidates_count, use_nlf))
            {
                mtf_candiScale = get_average(candidates_count, qVScale);
            }
            else
            {
                mtf_candiScale = 0;
            }
        }
        else
        {
            if (FilterVertices::MotifFilter_1step(data_graph, query_graph, candidates, candidates_count, use_nlf))
            {
                mtf_candiScale = get_average(candidates_count, qVScale);
            }
            else
            {
                mtf_candiScale = 0;
            }
        }

#ifdef DEBUG
        print_candidates(candidates, candidates_count, qVScale);
        std::cout << "this ave:" << mtf_candiScale << std::endl;
#endif //DEBUG
        moving_average(mtf_candiScale_ave, mtf_candiScale, cnt);
#ifdef DEBUG
        std::cout << "after moving, the new ave:" << mtf_candiScale_ave << std::endl;
#endif //DEBUG
        for (ui i = 0; i < qVScale; ++i)
        {
            delete[] candidates[i];
        }
        delete[] candidates;
        delete[] candidates_count;
        delete query_graph;
    }
}
#endif //TOPO_MOTIF_ENABLE==1

void write_time(ui q_vertexScale, std::string sparse_str, std::string result_fname_prefix, std::string dg_name, std::string filterTypeName, double duration)
{
    std::ofstream fout(result_fname_prefix + filterTypeName + "_Time.txt", std::ios::app);
    std::string split_c = " ";
    fout << dg_name << "\n";
    fout << q_vertexScale << split_c << sparse_str[0] << "\n";

    fout << duration << "\n\n";

    fout.close();
}

void print_scale_and_diff(double mtf_candiScale, std::string mtf_comment, double nlf_candiScale, std::string nlf_comment)
{
    char split_c = '\t';
    std::cout << mtf_comment << split_c << mtf_candiScale << std::endl;
    std::cout << nlf_comment << split_c << nlf_candiScale << std::endl;
    std::cout << "rate%:" << split_c << (nlf_candiScale - mtf_candiScale) / nlf_candiScale * 100 << std::endl;
}

void print_scale_and_diff_inner(std::vector<ui> q_vertexScale_set, double *mtf_ave_candiScale_arr, std::string mtf_comment, double *nlf_ave_candiScale_arr, std::string nlf_comment)
{
    char split_c = '\t';
    //q_vertexScale_set
    std::cout << "qVS:" << split_c;
    for (ui q_vertexScale : q_vertexScale_set)
    {
        std::cout << (double)q_vertexScale << split_c;
    }
    std::cout << std::endl;

    //mtf_ave_candiScale_arr
    std::cout << mtf_comment << split_c;
    ui i = 0;
    for (ui q_vertexScale : q_vertexScale_set)
    {
        std::cout << mtf_ave_candiScale_arr[i++] << split_c;
    }
    std::cout << std::endl;

    //nlf_ave_candiScale_arr
    std::cout << nlf_comment << split_c;
    i = 0;
    for (ui q_vertexScale : q_vertexScale_set)
    {
        std::cout << nlf_ave_candiScale_arr[i++] << split_c;
    }
    std::cout << std::endl;

    //rate%
    std::cout << "rate%" << split_c;
    i = 0;
    for (ui q_vertexScale : q_vertexScale_set)
    {
        std::cout << (nlf_ave_candiScale_arr[i] - mtf_ave_candiScale_arr[i]) / nlf_ave_candiScale_arr[i] * 100 << split_c;
        ++i;
    }
    std::cout << std::endl;
}

void print_scale_and_diff(std::vector<ui> q_vertexScale_set, double *mtf_ave_candiScale_arr, std::string mtf_comment, double *nlf_ave_candiScale_arr, std::string nlf_comment)
{
    ui col_per_page = 6;
    ui sz = q_vertexScale_set.size();
    ui e;
    std::cout << std::fixed << std::setprecision(2);
    for (ui i = 0; i < sz; i += col_per_page)
    {
        std::vector<ui> q_vertexScale_set_local;
        e = std::min(col_per_page, sz - i);
        for (ui j = 0; j < e; ++j)
        {
            q_vertexScale_set_local.push_back(q_vertexScale_set[i + j]);
        }
        print_scale_and_diff_inner(q_vertexScale_set_local, mtf_ave_candiScale_arr + i, mtf_comment, nlf_ave_candiScale_arr + i, nlf_comment);
        std::cout << std::endl;
    }
}

bool input_kmethod_set(std::string *&method_set, ui &KMETHOD_NUM, std::string *DEFAULT_KMETHOD_SET)
{
    std::string line;
    std::cout << "\ninput method_set followed by an enter, or just type enter for default:\nplease end with enter" << std::endl;
    std::cout << "[split with white space in one line, the choice must be the subset of {top down mid rand} (no order is needed)" << std::endl;
    std::cout << " eg: top down(enter)]" << std::endl;
    std::cout << "[if choose default: default method_set is DEFAULT_KMETHOD_SET in config.h]" << std::endl;
    std::getline(std::cin, line);
    if (line.empty())
    {
        method_set = DEFAULT_KMETHOD_SET;
        KMETHOD_NUM = DEFAULT_KMETHOD_NUM;
        return 0;
    }

    method_set = new std::string[DEFAULT_KMETHOD_NUM];
    ui i = 0;
    std::stringstream ss(line);
    while (ss >> line)
    {
        method_set[i++] = line;
    }
    KMETHOD_NUM = i;
    return 1; //need to release method_set
}

void input_kb_ke(ui &kb, ui &ke, ui min_lmtf_kind, ui max_lmtf_kind)
{
    std::string line;
    std::cout << "\ninput kb followed by an enter, or just type enter for default:" << std::endl;
    std::cout << "[if choose default: default kb and ke is the min and max of the kind num of lmtf of data_graph]" << std::endl;
    std::getline(std::cin, line);
    if (line.empty())
    {
        kb = min_lmtf_kind;
        ke = max_lmtf_kind;
    }
    else
    {
        kb = std::stoul(line);
        std::cout << "\ninput ke followed by an enter:" << std::endl;
        std::cin >> ke;
        getchar(); //for next getline()
    }
}

#endif //#if (DIRECTED_GRAPH == 1 || LABEL_MOTIF_ENABLE == 1)&& ONLINE_STAGE==1

void print_candidates(ui **candidates, ui *candidates_num, ui sz)
{
    ui num;
    char split_c = '\t';
    for (ui i = 0; i < sz; ++i)
    {
        num = candidates_num[i];
        std::cout << "u" << i + 1 << ":" << std::endl;
        for (ui j = 0; j < num; ++j)
        {
            std::cout << "v" << candidates[i][j] + 1 << split_c;
        }
        std::cout << std::endl;
    }
}

int getSysMaxOpenFilesNum()
{
    char line[30];
    FILE *fp;
    std::string cmd = "ulimit -n";
    const char *sysCommand = cmd.data();
    if ((fp = popen(sysCommand, "r")) == NULL)
    {
        std::cout << "get 'ulimit -n' failed" << std::endl;
        return -1;
    }
    std::string nofile_str;
    int nofile;
    while (fgets(line, sizeof(line) - 1, fp) != NULL)
    {
        nofile_str = std::string(line);
        nofile = std::stoi(nofile_str);
    }
    pclose(fp);
    return nofile;
}

void goto_first_of_last_line(std::ifstream &fin)
{
    fin.seekg(-1, std::ios_base::end);
    /*
    123456 [last line]
         ^(ios_base::cur points here)
    */
    //move to the first char in last line
    char c;
    while (1)
    {
        c = fin.get(); //move ios_base::cur one char further(therefore the seekg(-2) below)
        /*eg:in first loop
        123456 [last line]
              ^(ios_base::cur points here)
        */
        if (int(fin.tellg()) <= 1)
        {
            //only one line
            /*now
            123456 [the only line in fin]
             ^(ios_base::cur points here)
            seekg(-2) will be illegal
            */
            fin.seekg(std::ios_base::beg);
            break;
        }
        if (c == '\n')
        {
            break; //cur is ok (pointing to the char after this newline)
        }
        fin.seekg(-2, std::ios_base::cur);
    }
}
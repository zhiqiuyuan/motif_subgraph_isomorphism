#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <sstream>
#include <cfloat>
#include <time.h>
#include "inc/Graph.h"
#include "inc/FilterVertices.h"
#include "inc/FilterQueryHelp.h"

std::string DEFAULT_KMETHOD_SET[DEFAULT_KMETHOD_NUM] = {"down"}; //default
std::string *KMETHOD_SET;                                        //实际输入之后用这个
ui KMETHOD_NUM;

std::vector<unsigned> DEFAULT_QUERY_VERTEXSCALE_SET = {8, 16, 24, 32}; //q批量时若用户选择default则使用此作为q_vertexScale_set
                                                                       //实际输入之后用这个 //KMETHOD_NUM一定<=DEFAULT_KMETHOD_NUM（因为KMETHOD_SET必须是DEFAULT_KMETHOD_SET的子集）

#if ONLINE_STAGE == 0
/*
directed label
dfile_prefix:"xxx/youtube"，将调用generateMotifCount_label或generateMotifCount（需要传入"xxx/youtube"）
data_graph已经加载
*/
void offline_compute_motifStruct(std::string dfile_prefix, Graph *data_graph)
{
#if LABEL_MOTIF_ENABLE == 1
    std::cout << "compute label motif structure..." << std::endl;
#if DIRECTED_GRAPH == 1
#if LABEL_MOTIF_LIMIT == 0
    ui max_kind, min_kind;
    std::ifstream fin(dfile_prefix + "_directed_label_max_min.txt");
    fin >> max_kind >> min_kind;
#ifdef DEBUG
    std::cout << "lmtf_max_kind:" << max_kind << " lmtf_min_kind:" << min_kind << std::endl;
#endif //DEBUG
    fin.close();

    std::string kmethod_set[] = {"top", "down", "mid", "rand"};
    KMETHOD_NUM = 4;
    ui k = std::min(max_kind, ui(KMAX));
    ui vertexID_begin = 0, vertex_period = 5000;
    data_graph->generateMotifCount_label(dfile_prefix, vertexID_begin, vertex_period, k, k, kmethod_set, KMETHOD_NUM, 0);
#else  //LABEL_MOTIF_LIMIT==1:
    data_graph->generateMotifCount_label_limit(dfile_prefix);
#endif //#if LABEL_MOTIF_LIMIT==0
#ifdef DEBUG
//below is ILLEGAL: offline stage won't store result in memory, check file instead
//data_graph->print_graph_mtf(0); //use k as dime
#endif //DEBUG
#else
    data_graph->generateMotifCount_label(dfile_prefix);
#endif //DIRECTED_GRAPH == 1 //// LABEL_MOTIF_COUNT == 1 ////ONLINE_STAGE == 0
#endif // LABEL_MOTIF_COUNT == 1

#if TOPO_MOTIF_ENABLE == 1
    std::cout << "compute topo motif structure..." << std::endl;
    data_graph->generateMotifCount(dfile_prefix);
#endif //TOPO_MOTIF_ENABLE == 1
}

//标签有向图：统计所有q图中频率为1的特征占比
#if LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH == 1
int main_1(int argc, char **argv)
{
    std::string head = "/media/data/hnu2022/yuanzhiqiu/";
    std::vector<std::string> g1 = {"dblp", "eu2005", "hprd", "patents", "yeast", "youtube"};
    std::vector<std::string> g2 = {"human", "wordnet"};
    std::vector<std::string> sd = {"sparse_", "dense_"};
    std::vector<ui> qset1 = {8, 16, 24, 32};
    std::vector<ui> qset2 = {8, 12, 16, 20};
    ui jb = 1, je = 200;
    ui one_cnt_ = 0, total_cnt_ = 0;
    for (auto g : g1)
    {
        ui one_cnt = 0, total_cnt = 0;
        for (auto s : sd)
        {
            std::string fname = head + g + "/query_graph/query_" + s; //xxx/query_sparse_
            for (auto i : qset1)
            {
                std::string fname1 = fname + std::to_string(i) + "_"; //xxx/query_sparse_4_
                for (ui j = jb; j <= je; ++j)
                {
                    std::string fname2 = fname1 + std::to_string(j) + ".graph"; //xxx/query_sparse_4_j.graph
                    //std::cout << fname2 << std::endl;
                    Graph *query_graph = new Graph();
                    query_graph->loadGraphFromFile_directed(fname2);
                    query_graph->generateMotifCount_label(one_cnt, total_cnt);
                    one_cnt_ += one_cnt;
                    total_cnt_ += total_cnt;
                    delete query_graph;
                }
            }
        }
        std::cout << g << ":\n";
        std::cout << "one_cne:" << one_cnt << "\ntotal_cnt:" << total_cnt << std::endl;
        std::cout << "one_cnt/total_cnt:" << (double(one_cnt)) / total_cnt << "\n"
                  << std::endl;
    }
    for (auto g : g2)
    {
        ui one_cnt = 0, total_cnt = 0;
        for (auto s : sd)
        {
            std::string fname = head + g + "/query_graph/query_" + s; //xxx/query_sparse_
            for (auto i : qset2)
            {
                std::string fname1 = fname + std::to_string(i) + "_"; //xxx/query_sparse_4_
                for (ui j = jb; j <= je; ++j)
                {
                    std::string fname2 = fname1 + std::to_string(j) + ".graph"; //xxx/query_sparse_4_j.graph
                    //std::cout << fname2 << std::endl;
                    Graph *query_graph = new Graph();
                    query_graph->loadGraphFromFile_directed(fname2);
                    query_graph->generateMotifCount_label(one_cnt, total_cnt);
                    one_cnt_ += one_cnt;
                    total_cnt_ += total_cnt;
                    delete query_graph;
                }
            }
        }
        std::cout << g << ":\n";
        std::cout << "one_cne:" << one_cnt << "\ntotal_cnt:" << total_cnt << std::endl;
        std::cout << "one_cnt/total_cnt:" << (double(one_cnt)) / total_cnt << "\n"
                  << std::endl;
    }

    std::cout << "one_cne_:" << one_cnt_ << "\ntotal_cnt_:" << total_cnt_ << std::endl;
    std::cout << "one_cnt/total_cnt:" << (double(one_cnt_)) / total_cnt_ << std::endl;
    return 0;
}
#endif //#if LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH==1

//输出指定数据图的性质
int main_2()
{
    std::string fnamepre = "/media/data/hnu2022/yuanzhiqiu/";
    std::vector<std::string> dg = {"human", "wordnet", "dblp", "eu2005", "youtube", "patents"};
    for (auto dn : dg)
    {
        std::string fname = fnamepre + dn + "/data_graph/" + dn + ".graph";
        Graph *g = new Graph();
        g->loadGraphFromFile_directed(fname);
        std::cout << "数据图属性：" << std::endl;
        std::cout << "| \\|顶点数\\| | \\|边数\\| | \\|顶点标签集\\| | 顶点平均度数（入度+出度） |" << std::endl;
        std::cout << "| ---------- | -------- | -------------- | ------------------------- |" << std::endl;
        std::cout << "| " << g->getVerticesCount() << " |" << g->getEdgesCount() << " |" << g->getLabelsCount() << " |\n"
                  << std::endl;
    }
    return 0;
}

//输出图中的bi degree之和
int main_3(int argc, char **argv)
{
    std::string filename = argv[1];
    Graph *data_graph = new Graph();
    data_graph->loadGraphFromFile_directed(filename);
    ui vertices_cnt = data_graph->getVerticesCount();
    ui total_bidegree = 0;
    for (ui i = 0; i < vertices_cnt; ++i)
    {
        total_bidegree += data_graph->getVertexBiDegree(i);
    }
    std::string dgname = filename.substr(filename.find_last_of('/') + 1);
    std::cout << dgname << ":" << total_bidegree << std::endl;
    return 0;
}

#endif //#if ONLINE_STAGE==0

int main(int argc, char **argv)
{
    /*
    GET CONSOLE OPTIONS
    */
    std::string input_data_graph_file, input_query_graph_file, filter_type; //CONFIG
    bool label;
    //for bulk
    std::vector<ui> q_vertexScale_set = DEFAULT_QUERY_VERTEXSCALE_SET;
    ui jb = DEFAULT_JB, je = DEFAULT_JE;
    ui jsz = je - jb + 1;

    //-d -q -f (lmtf,lmtf1s, mtf, mtf_nlf, mtf1s, mtf1s_nlf, nlf, nlf1s, ldf)
    if (argc < 7)
    {
        std::cout << "usage:" << std::endl;
        std::cout << "-d data_graph_absolute_path -q query_graph_absolute_path -f (lmtf,lmtf1s, mtf, mtf_nlf, mtf1s, mtf1s_nlf, nlf, nlf1s, ldf) -qset 8 12" << std::endl;
        std::cout << "[example]: \n";
        std::cout << "./main -d /media/data/hnu2022/yuanzhiqiu/human/data_graph/human.graph -q /media/data/hnu2022/yuanzhiqiu/human/query_graph/SPARSE -f lmtf" << std::endl;
        return 0;
    }
    std::string op, val;
    ui dqf_para_num = 7;
    std::map<std::string, std::string> cmd;
    if (argc == 7)
    {
        for (int i = 1; i < dqf_para_num; i += 2)
        {
            op = argv[i];
            val = argv[i + 1];
            cmd[op] = val;
        }
        input_data_graph_file = cmd["-d"];
        input_query_graph_file = cmd["-q"];
        filter_type = cmd["-f"];
    }
    else
    { //input q_vertexScale_set
        q_vertexScale_set.clear();
        for (int i = 1; i < argc;)
        {
            op = argv[i];
            if (op == "-qset")
            {
                ++i;
                while (i < argc && argv[i][0] != '-')
                {
                    q_vertexScale_set.push_back(std::stoi(std::string(argv[i])));
                    ++i;
                }
                //argv[i][0]=='-'||i>=argc
            }
            else
            {
                val = argv[i + 1];
                cmd[op] = val;
                i += 2;
            }
        }
        input_data_graph_file = cmd["-d"];
        input_query_graph_file = cmd["-q"];
        filter_type = cmd["-f"];
    }
#ifdef ONLINE_DEBUG
    for (auto it : cmd)
    {
        std::cout << it.first << ":" << it.second << std::endl;
    }
    std::cout << "q_vertexScale_set:" << std::endl;
    for (ui i : q_vertexScale_set)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
#endif //ONLINE_DEBUG

    /*
    LOAD DATA GRAPH
    */
    std::cout << "load data graph..." << std::endl;
    Graph *data_graph = new Graph();
#if DIRECTED_GRAPH == 1
    data_graph->loadGraphFromFile_directed(input_data_graph_file);
#else
    data_graph->loadGraphFromFile(input_data_graph_file);
#endif // DIRECTED_GRAPH
    std::cout << "-----" << std::endl;
    std::cout << "data Graph Meta Information" << std::endl;
    data_graph->printGraphMetaData();
    std::cout << "-----" << std::endl;
#ifdef DEBUG
    //data_graph->print_graph_detail();
    //std::cout << "-----" << std::endl;
#endif //DEBUG
    size_t idx = input_data_graph_file.find_last_of(".");
    std::string input_data_graph_file_prefix = input_data_graph_file.substr(0, idx); //xxx/youtube（没有.graph）

    idx = input_data_graph_file_prefix.find_last_of('/');
    std::string dg_name = input_data_graph_file_prefix.substr(idx + 1); //youtube

    idx = input_data_graph_file_prefix.substr(0, idx).find(dg_name);
    std::string result_fname_prefix = input_data_graph_file_prefix.substr(0, idx) + "filter_candiScale/"; // /media/data/hnu2022/yuanzhiqiu/filter_candiScale/

#if ONLINE_STAGE == 0 //offline
    offline_compute_motifStruct(input_data_graph_file_prefix, data_graph);
#else                                                                          // online
    std::string sparse_str;
    std::string line;

    idx = input_query_graph_file.find_last_of('/');
    sparse_str = input_query_graph_file.substr(idx + 1); //SPARSE
    /*
    youtube
    qVScale1 S/D
    candiScale

    /media/data/hnu2022/yuanzhiqiu/filter_candiScale/xxx_CandiScale.txt
    xxx:method
    */
    if (filter_type.find("mtf") != std::string::npos)
    {
        /*
        LOAD MOTIF STRUCTURE FOR DATA GRAPH
        */
        if (filter_type == "lmtf" || filter_type == "lmtf1s" || filter_type == "lmtf_limit")
        {
#if (LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH == 1 && LABEL_MOTIF_LIMIT == 0) //directed_label
            if (filter_type == "lmtf_limit")
            {
                std::cout << "LABEL_MOTIF_LIMIT:" << LABEL_MOTIF_LIMIT << std::endl;
                std::cout << "LABEL_MOTIF_LIMIT == 1 is needed, modify inc/config.h" << std::endl;
                return 0;
            }
            //min,max lmtf kind
            std::ifstream fin(input_data_graph_file_prefix + "_directed_label_max_min.txt");
            ui max_lmtf_kind, min_lmtf_kind;
            fin >> max_lmtf_kind >> min_lmtf_kind;
            fin.close();
#ifdef DEBUG
            std::cout << "lmtf_max_kind:" << max_lmtf_kind << " lmtf_min_kind:" << min_lmtf_kind << std::endl;
#endif //DEBUG
            ui kInFileName = std::min(max_lmtf_kind, ui(KMAX));
#ifdef DEBUG
            std::cout << "kInFileName(use to construct filename to load data_graph labelmotif structure from):" << kInFileName << std::endl;
#endif //DEBUG
            /*========================
            CONFIG HERE
            ========================*/
            ui kb = kInFileName, ke = kInFileName;
            std::string *method_set = DEFAULT_KMETHOD_SET;
            KMETHOD_NUM = DEFAULT_KMETHOD_NUM;
            bool one_step = (filter_type == "lmtf1s") ? 1 : 0;
            if (sparse_str == "SPARSE" || sparse_str == "DENSE")
            { //BULK
#if COLLECT_DATA_FEATURE == 1
                ui feature[4] = {0};
#endif // #if COLLECT_DATA_FEATURE == 1 \
       //double nlf_ave_candiScale;
                kMethodCandiScale *lmtf_ave_candiScale_arr; //each (k, kmethod) pair an entry
                for (ui q_vertexScale : q_vertexScale_set)
                {
                    //bulkq_nlf_filterCandi(input_query_graph_file, nlf_ave_candiScale, data_graph, q_vertexScale, jb, je);
#ifdef DEBUG
                    std::cout << "nlf_ave_candiScale:" << nlf_ave_candiScale << std::endl;
#endif //DEBUG
                    auto start = std::chrono::system_clock::now();
                    //clock_t cstart = clock();
#if COLLECT_DATA_FEATURE == 1
                    bulkq_dLmtf_filterCandi_vary_k_method_collect_data_feature(input_data_graph_file_prefix, input_query_graph_file, lmtf_ave_candiScale_arr, kb, ke, kInFileName, method_set, KMETHOD_NUM, data_graph, q_vertexScale, jb, je, feature);
#else
                    bulkq_dLmtf_filterCandi_vary_k_method(input_data_graph_file_prefix, input_query_graph_file, lmtf_ave_candiScale_arr, kb, ke, kInFileName, method_set, KMETHOD_NUM, data_graph, q_vertexScale, jb, je, one_step);
#endif //COLLECT_DATA_FEATURE==1
                    auto end = std::chrono::system_clock::now();
                    //clock_t cend = clock();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                    double d = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
                    d = d / (jsz * (ke - kb + 1));
                    /*
                    double tmp = double(duration.count());
                    double d;
                    ui sz = (ke - kb + 1) * KMETHOD_NUM; //size of lmtf_ave_candiScale_arr;

                    if (tmp > DBL_MAX || tmp < DBL_MIN)
                    {
                        //overflow, use clock
                        d = (double)(cend - cstart) / CLOCKS_PER_SEC;
                        if (d > DBL_MAX || d < DBL_MIN)
                        {
                            //overflow(well...)
                            d = ((double)cend / CLOCKS_PER_SEC) - ((double)cstart / CLOCKS_PER_SEC);
                            if (d > DBL_MAX || d < DBL_MIN)
                            {
                                d = (((double)cend / (sz * jsz)) / CLOCKS_PER_SEC) - (((double)cstart / (sz * jsz)) / CLOCKS_PER_SEC);
                            }
                            else
                            {
                                d /= (sz * jsz);
                            }
                        }
                        else
                        {
                            d /= (sz * jsz);
                        }
                    }
                    else
                    {
                        d = (std::chrono::microseconds::period::num / std::chrono::microseconds::period::den) * tmp;
                        if (d > DBL_MAX || d < DBL_MIN)
                        {
                            //d overflow but tmp not
                            d = (tmp / (sz * jsz)) * (std::chrono::microseconds::period::num / std::chrono::microseconds::period::den);
                        }
                        else
                        {
                            d /= (sz * jsz);
                        }
                    }
                    */
                    std::cout << "finish q_vertexScale:" << q_vertexScale << std::endl;
                    std::cout << "[max min] lmtf_kind of data_graph:[" << max_lmtf_kind << " " << min_lmtf_kind << "]" << std::endl;
//sz = 12;
#if COLLECT_DATA_FEATURE == 0
                    write_lmtf_CandiScale(lmtf_ave_candiScale_arr, method_set, sz, q_vertexScale, sparse_str, result_fname_prefix, dg_name, filter_type, d);
#else
                    write_time(q_vertexScale, sparse_str, result_fname_prefix, dg_name, filter_type, d);
#endif //#if COLLECT_DATA_FEATURE == 1 \
       //print_scale_and_diff(nlf_ave_candiScale, "nlf", lmtf_ave_candiScale_arr, "dlmtf", method_set, sz);
                    delete[] lmtf_ave_candiScale_arr;
                }
                data_graph->alloc_label_motif_count(); //因为在函数里面都释放掉了motif结构，下面delete data_graph就会出错，所以这里申请一下
#if COLLECT_DATA_FEATURE == 1
                std::ofstream foutf(result_fname_prefix + "feature.txt", std::ios::app);
                foutf << dg_name << " " << sparse_str << std::endl;
                for (ui kk = 0; kk < 4; ++kk)
                {
                    foutf << feature[kk] << " ";
                }
                foutf << std::endl;
                foutf.close();
#endif // #if COLLECT_DATA_FEATURE == 1
            }
#elif (LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH == 1 && LABEL_MOTIF_LIMIT == 1)
            if (filter_type == "lmtf_limit")
            {
                std::cout << "load all label motif structure for data graph..." << std::endl;
                data_graph->loadMotifCountFromFile_label_limit(input_data_graph_file_prefix);
#ifdef DEBUG
                std::cout << "label motif of data_graph:" << std::endl;
                data_graph->print_label_motif_map();
#endif //DEBUG
                std::ofstream fout(result_fname_prefix + filter_type + "_CandiScale.txt", std::ios::app);
                if (sparse_str == "SPARSE" || sparse_str == "DENSE")
                { //BULK
                    ui sz = q_vertexScale_set.size();
                    double mtf_ave_candiScale;
                    for (ui q_vertexScale : q_vertexScale_set)
                    {
                        std::string split_c = " ";
                        fout << dg_name << "\n";
                        fout << q_vertexScale << split_c << sparse_str[0] << "\n";
                        auto start = std::chrono::system_clock::now();
                        bulkq_lmtf_limit_filter(input_query_graph_file, data_graph, q_vertexScale, jb, je, mtf_ave_candiScale);
                        auto end = std::chrono::system_clock::now();

                        std::cout << "finish q_vertexScale:" << q_vertexScale << std::endl;
                        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                        double d = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
                        fout << mtf_ave_candiScale << "\n"
                             << d / jsz << "\n\n";
                    }
                }
            }

#else
            std::cout << "LABEL_MOTIF_ENABLE:" << LABEL_MOTIF_ENABLE << " DIRECTED_GRAPH:" << DIRECTED_GRAPH << std::endl;
            std::cout << "LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH == 1 is needed, modify inc/config.h" << std::endl;
#endif // (LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH == 1)
        }
        else
        { //mtf, mtf_nlf, mtf1s, mtf1s_nlf
#if TOPO_MOTIF_ENABLE == 1
            std::cout << "load topo motif structure for data graph..." << std::endl;
            data_graph->loadMotifCountFromFile(input_data_graph_file_prefix);
            std::ofstream fout(result_fname_prefix + filter_type + "_CandiScale.txt", std::ios::app);
            bool do_one_step = (filter_type.find("1s") == std::string::npos) ? 0 : 1;
            bool use_nlf = (filter_type.find("nlf") == std::string::npos) ? 0 : 1;
            if (sparse_str == "SPARSE" || sparse_str == "DENSE")
            { //BULK
                ui sz = q_vertexScale_set.size();
                double mtf_ave_candiScale;
                for (ui q_vertexScale : q_vertexScale_set)
                {
                    std::string split_c = " ";
                    fout << dg_name << "\n";
                    fout << q_vertexScale << split_c << sparse_str[0] << "\n";
                    auto start = std::chrono::system_clock::now();
                    bulkq_mtf_filter(input_query_graph_file, data_graph, q_vertexScale, jb, je, mtf_ave_candiScale, do_one_step, use_nlf);
                    auto end = std::chrono::system_clock::now();

                    std::cout << "finish q_vertexScale:" << q_vertexScale << std::endl;
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                    double d = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
                    fout << mtf_ave_candiScale << "\n"
                         << d / jsz << "\n\n";
                }
            }
#else
            std::cout << "TOPO_MOTIF_ENABLE:" << TOPO_MOTIF_ENABLE << std::endl;
            std::cout << "TOPO_MOTIF_ENABLE == 1 is needed, modify inc/config.h" << std::endl;
#endif //TOPO_MOTIF_ENABLE == 1
        }
    }
    else if (filter_type == "nlf" || filter_type == "ldf" || filter_type == "nlf1s")
    {
        if (sparse_str == "SPARSE" || sparse_str == "DENSE")
        { //BULK
            double nlf_ave_candiScale;
            std::ofstream fout(result_fname_prefix + filter_type + "_CandiScale.txt", std::ios::app);
            for (ui q_vertexScale : q_vertexScale_set)
            {
                std::string split_c = " ";
                fout << dg_name << "\n";
                fout << q_vertexScale << split_c << sparse_str[0] << "\n";
                auto start = std::chrono::system_clock::now();
                if (filter_type == "nlf")
                {
                    bulkq_nlf_filterCandi(input_query_graph_file, nlf_ave_candiScale, data_graph, q_vertexScale, jb, je, 0);
                }
                else if (filter_type == "nlf1s")
                {
                    bulkq_nlf_filterCandi(input_query_graph_file, nlf_ave_candiScale, data_graph, q_vertexScale, jb, je, 1);
                }
                else
                {
                    bulkq_ldf_filterCandi(input_query_graph_file, nlf_ave_candiScale, data_graph, q_vertexScale, jb, je);
                }
                auto end = std::chrono::system_clock::now();

                std::cout << "finish q_vertexScale:" << q_vertexScale << std::endl;
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                double d = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
                fout << nlf_ave_candiScale << "\n"
                     << d / jsz << "\n\n";
            }
            fout.close();
        }
    }
    else
    {
        std::cout << "filter_type not supported:" << filter_type << std::endl;
        std::cout << "supported filter_type: [lmtf,lmtf1s, mtf, mtf_nlf, mtf1s, mtf1s_nlf, nlf, nlf1s, ldf,lmtf_limit]" << std::endl;
    }

#endif //ONLINE_STAGE == 0 ELSE
    delete data_graph;
    //system("pause");
    return 0;
}

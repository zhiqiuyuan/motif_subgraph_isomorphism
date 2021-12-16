#include "GraphFeatures.h"
/*
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
    unsigned jb = 1, je = 200;
    unsigned one_cnt_ = 0, total_cnt_ = 0;
    for (auto g : g1)
    {
        unsigned one_cnt = 0, total_cnt = 0;
        for (auto s : sd)
        {
            std::string fname = head + g + "/query_graph/query_" + s; //xxx/query_sparse_
            for (auto i : qset1)
            {
                std::string fname1 = fname + std::to_string(i) + "_"; //xxx/query_sparse_4_
                for (unsigned j = jb; j <= je; ++j)
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
        unsigned one_cnt = 0, total_cnt = 0;
        for (auto s : sd)
        {
            std::string fname = head + g + "/query_graph/query_" + s; //xxx/query_sparse_
            for (auto i : qset2)
            {
                std::string fname1 = fname + std::to_string(i) + "_"; //xxx/query_sparse_4_
                for (unsigned j = jb; j <= je; ++j)
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
    unsigned vertices_cnt = data_graph->getVerticesCount();
    unsigned total_bidegree = 0;
    for (unsigned i = 0; i < vertices_cnt; ++i)
    {
        total_bidegree += data_graph->getVertexBiDegree(i);
    }
    std::string dgname = filename.substr(filename.find_last_of('/') + 1);
    std::cout << dgname << ":" << total_bidegree << std::endl;
    return 0;
}
*/
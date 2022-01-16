#include "TVEFileG.h"
#include "TVEFileQ.h"
#include "../bulkQueries/BulkQueries.h"

void printUsage(std::set<std::string> filterMethodset, std::string default_output_dir, std::vector<unsigned> DEFAULT_QUERY_VERTEXSCALE_SET)
{
    std::cout << "usage:" << std::endl;
    std::cout << "-d data_graph_absolute_path -q query_graph_absolute_path -f <filterMethod>(";
    for (auto s : filterMethodset)
    {
        std::cout << s << " ";
    }
    std::cout << ") [-o <candiScale结果追加写入的目录绝对路径>] [-qset <空格分隔的整数>] [-jb <jb>] [-je <je>]" << std::endl;
    std::cout << "[example]: \n";
    std::cout << "./FilterWriteCandiScale -d /media/data/hnu2022/yuanzhiqiu/human/data_graph/human.graph -q /media/data/hnu2022/yuanzhiqiu/human/query_graph/SPARSE -f lmtf" << std::endl;
    std::cout << "default:\n\t-o " + default_output_dir;
    std::cout << "\n\t-qset ";
    for (auto num : DEFAULT_QUERY_VERTEXSCALE_SET)
    {
        std::cout << num << " ";
    }
    std::cout << "\n\t-jb " << DEFAULT_JB << " -je " << DEFAULT_JE << std::endl;
}

/* WARNNING:没有检查命令行输入的option是否合法（即默认用户输入的option是支持的option的子集），仅检查输入的-f是否是合理取值
*/
/* 命令行参数：
* -d <data_graph_absolute_path> 
* -q <query_graph_absolute_path> 
* -f <filterMethod> (目前支持："ldf","tmtf","nlf","lmtf_limit","gql")
* -o <candiScale结果追加写入的目录绝对路径> (可选，默认std::string default_output_dir)
* -qset <空格分隔的正整数> (可选，默认{8, 16, 24, 32})
* -jb <jb> (可选，默认config.h中DEFAULT_JB)
* -je <je> (可选，默认config.h中DEFAULT_JE)
* 
* candiScale和time结果追加写
*/
/* candiScale结果追加写入文件：<candiScale结果追加写入的目录绝对路径>xxx_CandiScale.txt
其中xxx:method

格式：
youtube
qVScale S/D
candiScale
time
*/
int main(int argc, char **argv)
{
#if ONLINE_STAGE == 0
    std::cout << "ONLINE_STAGE:" << ONLINE_STAGE << std::endl;
    std::cout << "ONLINE_STAGE == 1 is needed, modify config.h" << std::endl;
    return 0;
#endif //#if ONLINE_STAGE==0

    /* GET CONSOLE OPTIONS
    */
    std::vector<unsigned> DEFAULT_QUERY_VERTEXSCALE_SET = {8, 16, 24, 32};
    std::set<std::string> filterMethodset = {"ldf", "tmtf", "nlf", "lmtf_limit", "gql", "cfl", "dpiso",
                                             "nlf_lmtf_limit", "gql_lmtf_limit", "cfl_lmtf_limit", "dpiso_lmtf_limit"};

    std::string input_data_graph_file, input_query_graph_file, filterMethod;
    std::string default_output_dir = "/media/data/hnu2022/yuanzhiqiu/running_result/filterWriteCandiScale_result/default_output_dir/"; //默认值
    std::string output_dir = default_output_dir;
    std::vector<unsigned> q_vertexScale_set = DEFAULT_QUERY_VERTEXSCALE_SET; //默认值
    unsigned jb = DEFAULT_JB, je = DEFAULT_JE;                               //默认值
    unsigned jsz = je - jb + 1;

    if (argc < 7)
    {
        printUsage(filterMethodset, default_output_dir, DEFAULT_QUERY_VERTEXSCALE_SET);
        return 0;
    }
    std::string op, val;
    unsigned dqf_para_num = 7;
    std::map<std::string, std::string> cmd;
    //qset和candiScale结果追加写入的目录绝对路径都是默认
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
        filterMethod = cmd["-f"];
    }
    //有-qset或-o或jb或je
    else
    {
        for (int i = 1; i < argc;)
        {
            op = argv[i];
            if (op == "-qset")
            {
                q_vertexScale_set.clear();
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
        filterMethod = cmd["-f"];
        if (cmd.count("-o"))
        {
            output_dir = cmd["-o"];
        }
        if (cmd.count("-jb"))
        {
            jb = std::stoul(cmd["-jb"]);
            jsz = je - jb + 1;
        }
        if (cmd.count("-je"))
        {
            je = std::stoul(cmd["-je"]);
            jsz = je - jb + 1;
        }
    }

    if (filterMethodset.count(filterMethod) == 0)
    {
        std::cout << "filterMethod NOT SUPPORTED: " << filterMethod << "\n";
        std::cout << "supported <filterMethod>(";
        for (auto s : filterMethodset)
        {
            std::cout << s << " ";
        }
        std::cout << ")\n";
        return 0;
    }
#if STEP_DEBUG == 1
    /* 输出cmd map，检查cmd map对不对
    for (auto it : cmd)
    {
        std::cout << it.first << ":" << it.second << std::endl;
    }*/
    std::cout << "filterMethod:" << filterMethod << "\nq_vertexScale_set:" << std::endl;
    for (unsigned i : q_vertexScale_set)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    std::cout << "jb:" << jb << " je:" << je << "\noutput_dir:" << output_dir << std::endl;
#endif //#if STEP_DEBUG==1

    /* PREPARE DATA GRAPH: load basic structure
    */
#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
    std::cout << "load data graph basic structure..." << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1
    TVEFileG *data_graph = new TVEFileG();
    data_graph->loadGraphFromFile(input_data_graph_file);

#if RUNNING_COMMENT == 1
    std::cout << "-----" << std::endl;
    std::cout << "data Graph Meta Information" << std::endl;
    data_graph->printGraphMetaData();
    std::cout << "-----" << std::endl;
#endif //#if RUNNING_COMMENT==1

#if STEP_DEBUG == 1
    //data_graph->printGraphDetail(0);
    //std::cout << "-----" << std::endl;
#endif //#if STEP_DEBUG == 1

    /* PREPARE DATA GRAPH: load offline structure
    */
    size_t idx = input_data_graph_file.find_last_of(".");
    std::string input_data_graph_file_prefix = input_data_graph_file.substr(0, idx); //xxx/youtube

    /* WARNNING: tmtf硬编码使用nlf，所以使用tmtf需要配置TOPO_MOTIF_ENABLE == 1 && LABEL_MOTIF_ENABLE == 1
    */
    if (filterMethod == "tmtf")
    {
#if TOPO_MOTIF_ENABLE == 1 && LABEL_MOTIF_ENABLE == 1
#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
        std::cout << "compute nlf structure for data graph..." << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1
        data_graph->BuildNLF();
#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
        std::cout << "load topo motif structure for data graph..." << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1
        data_graph->loadTopoMotifCountFromFile(input_data_graph_file_prefix);

#else  //TOPO_MOTIF_ENABLE == 0 || LABEL_MOTIF_ENABLE==0
        std::cout << "TOPO_MOTIF_ENABLE:" << TOPO_MOTIF_ENABLE << std::endl;
        std::cout << "LABEL_MOTIF_ENABLE:" << LABEL_MOTIF_ENABLE << std::endl;
        std::cout << "TOPO_MOTIF_ENABLE == 1 && LABEL_MOTIF_ENABLE == 1(the latter for nlf) is needed, modify config.h" << std::endl;
        return 0;
#endif //TOPO_MOTIF_ENABLE == 1 && LABEL_MOTIF_ENABLE==1
    }
    /*如果GQL之前用的labelmotif_limit则|| filterMethod == "gql"放lmtf_limit分支 
    CFL同,DPiso同*/
    else if (filterMethod == "nlf" || filterMethod == "gql" || filterMethod == "cfl")
    {
#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
        std::cout << "compute nlf structure for data graph..." << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1
        data_graph->BuildNLF();
    }
    /*如果GQL之前用的labelmotif_limit则|| filterMethod == "gql"放这个分支 
    CFL,DPiso同*/
    else if (filterMethod == "lmtf_limit" || filterMethod.find("_lmtf_limit") != std::string::npos)
    {
#if LABEL_MOTIF_LIMIT == 1 && LABEL_MOTIF_ENABLE == 1
#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
        std::cout << "compute nlf structure for data graph..." << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1
        data_graph->BuildNLF();
#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
        std::cout << "load label motif structure(limit) for data graph..." << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1
        data_graph->loadLabelMotifCountFromFile_limit(input_data_graph_file_prefix);

#else  //TOPO_MOTIF_ENABLE == 0 || LABEL_MOTIF_ENABLE==0
        std::cout << "LABEL_MOTIF_LIMIT:" << LABEL_MOTIF_LIMIT << std::endl;
        std::cout << "LABEL_MOTIF_ENABLE:" << LABEL_MOTIF_ENABLE << std::endl;
        std::cout << "LABEL_MOTIF_LIMIT == 1 && LABEL_MOTIF_ENABLE == 1(the latter for nlf) is needed, modify config.h" << std::endl;
        return 0;
#endif //LABEL_MOTIF_LIMIT == 1 && LABEL_MOTIF_ENABLE==1
    }

    /* FILTER
    */
#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
    std::cout << "start filtering..." << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1
    idx = input_query_graph_file.find_last_of('/');
    std::string sparse_str = input_query_graph_file.substr(idx + 1); //SPARSE
    idx = input_data_graph_file_prefix.find_last_of('/');
    std::string dg_name = input_data_graph_file_prefix.substr(idx + 1); //youtube

    BulkQueries *query = new BulkQueries(input_query_graph_file, data_graph, jb, je);

    std::ofstream fout(output_dir + filterMethod + "_CandiScale.txt", std::ios::app);
    unsigned sz = q_vertexScale_set.size();
    for (unsigned q_vertexScale : q_vertexScale_set)
    {
        query->setQVScale(q_vertexScale);
        std::string split_c = " ";
        fout << dg_name << "\n";
        fout << q_vertexScale << split_c << sparse_str[0] << "\n";

        auto start = std::chrono::system_clock::now();
        query->bulkq_filter_forAveCandiScale(filterMethod);
        auto end = std::chrono::system_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double d = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
        fout << query->get_ave_candiScale() << "\n"
             << d / jsz << "\n\n";

#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
        std::cout << "finish q_vertexScale:" << q_vertexScale << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1
    }

    delete data_graph;
    delete query;
    return 0;
}

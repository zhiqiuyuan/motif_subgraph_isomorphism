#include "TVEFileG.h"
#include "TVEFileQ.h"
#include "../bulkQueries/BulkQueries.h"

void printUsage(std::string default_output_dir, std::vector<unsigned> DEFAULT_QUERY_VERTEXSCALE_SET)
{
    std::cout << "usage:" << std::endl;
    std::cout << "-q query_graph_absolute_path";
    std::cout << " [-o <Qfeatures结果覆盖写入的目录绝对路径>] [-qset <空格分隔的整数>] [-jb <jb>] [-je <je>]" << std::endl;
    std::cout << "[example]: \n";
    std::cout << "./<bin_name> -q /media/data/hnu2022/yuanzhiqiu/human/query_graph/SPARSE" << std::endl;
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
* -q <query_graph_absolute_path> 
* -o WARNNING:尚未实现<Qfeatures结果覆盖写入的目录绝对路径> (可选，默认std::string default_output_dir)
* -qset <空格分隔的正整数> (可选，默认{8, 16, 24, 32})
* -jb <jb> (可选，默认config.h中DEFAULT_JB)
* -je <je> (可选，默认config.h中DEFAULT_JE)
* 
*/

int main(int argc, char **argv)
{
    /* GET CONSOLE OPTIONS
    */
    std::vector<unsigned> DEFAULT_QUERY_VERTEXSCALE_SET = {8, 16, 24, 32};

    std::string input_data_graph_file, input_query_graph_file, filterMethod;
    std::string default_output_dir = "/media/data/hnu2022/yuanzhiqiu/running_result/filterWriteCandiScale_result/default_output_dir/"; //默认值
    std::string output_dir = default_output_dir;
    std::vector<unsigned> q_vertexScale_set = DEFAULT_QUERY_VERTEXSCALE_SET; //默认值
    unsigned jb = DEFAULT_JB, je = DEFAULT_JE;                               //默认值
    unsigned jsz = je - jb + 1;

    if (argc < 3)
    {
        printUsage(default_output_dir, DEFAULT_QUERY_VERTEXSCALE_SET);
        return 0;
    }
    std::string op, val;
    unsigned dqf_para_num = 3;
    std::map<std::string, std::string> cmd;
    //qset和candiScale结果追加写入的目录绝对路径都是默认
    if (argc == 3)
    {
        for (int i = 1; i < dqf_para_num; i += 2)
        {
            op = argv[i];
            val = argv[i + 1];
            cmd[op] = val;
        }
        input_query_graph_file = cmd["-q"];
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
        input_query_graph_file = cmd["-q"];
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

#if STEP_DEBUG == 1
    /* 输出cmd map，检查cmd map对不对
    for (auto it : cmd)
    {
        std::cout << it.first << ":" << it.second << std::endl;
    }*/
    std::cout << "q_vertexScale_set:" << std::endl;
    for (unsigned i : q_vertexScale_set)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    std::cout << "jb:" << jb << " je:" << je << "\noutput_dir:" << output_dir << std::endl;
#endif //#if STEP_DEBUG==1

    BulkQueries query(input_query_graph_file, 0, jb, je);
    for (unsigned q_vertexScale : q_vertexScale_set)
    {
        query.setQVScale(q_vertexScale);
        std::cout << query.qset_is_weakConnected() << std::endl;
    }

    return 0;
}
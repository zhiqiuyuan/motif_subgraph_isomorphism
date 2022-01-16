#include "../util/GenerateTVEFileQ.h"
#include "TVEFileG.h"

void printUsage()
{
    std::cout << "usage:\n ./generateTVEFileQ -d dg_file_path [-v vcnt_arr] [-S sparse_arr] [-j qcnt_required] [-o output_dir]" << std::endl;
    std::cout << "[example]: \n";
    std::cout << "./generateTVEFileQ -d /media/data/hnu2022/yuanzhiqiu/human/data_graph/human.graph -v 8 16 24 32 -S 0 1 -j 100 -o /media/data/hnu2022/yuanzhiqiu/human/query_graph/" << std::endl;
    std::cout << "default:\n\t-v 8" << std::endl;
    std::cout << "\t-S 0" << std::endl;
    std::cout << "\t-j 200" << std::endl;
    std::cout << "\t-o <dg_file_path所在目录 ../../query_graph/'>" << std::endl;
    std::cout << "config usage:\n\tWRITE_TO_FILE_DEBUG==0则以t v e格式写入文件，==1则以便于debug的格式写入文件output_dir/query_dense_vcnt_currqcnt.graph\n\tCOLLECT_Q_FEATURE==1则会在生成查询图同时统计查询图特征，（追加写模式）写入output_dir的../properties_of_query_graph.txt\n";
}

/* 命令行参数：
* -d <数据图绝对路径>
* -v <可选，空格分隔vcnt_arr，生成查询图的顶点数>(默认：{8})
* -S <可选，空格分隔sparse_arr，取值{0} {1} {0 1}>(默认：{0})
* -j <可选，qcnt_required，每个qset需要的查询图数目>(默认：200)
* -o <可选，output_dir绝对路径>(默认：由"数据图绝对路径"推出的与data_graph目录同级的query_graph)
*
* WRITE_TO_FILE_DEBUG==0则以t v e格式写入文件，==1则以便于debug的格式写入文件output_dir/query_dense_vcnt_currqcnt.graph
* COLLECT_Q_FEATURE==1则会在生成查询图同时统计查询图特征，（追加写模式）写入output_dir的../Qfeatures.txt
*/
int main(int argc, char **argv)
{
    srand((unsigned)time(NULL));

    std::string dg_file_path;             //数据图绝对路径，xxx/data_graph/youtube.graph
    std::vector<unsigned> vcnt_arr = {8}; //默认值
    std::vector<bool> sparse_arr = {0};   //默认值
    unsigned qcnt_required = 200;         //默认值
    std::string dest_dir;

    if (argc < 3)
    {
        printUsage();
        return 0;
    }
    int index = 1;
    while (index < argc)
    {
        std::string argstr = argv[index];
        if (argstr[0] == '-')
        {
            char c = argstr[1];
            if (c == 'd') //dg_file_path
            {
                ++index;
                dg_file_path = argv[index];
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
            else if (c == 'o') //output_dir
            {
                ++index;
                dest_dir = argv[index];
                ++index;
            }
            else
            {
                std::cout << "not supported option: " << c << std::endl;
                printUsage();
                return 0;
            }
        }
        else
        {
            std::cout << "error input." << std::endl;
            printUsage();
            return 0;
        }
    }
    TVEFileG *dg = new TVEFileG();
    dg->loadGraphFromFile(dg_file_path);
    int GAP = dg->getVerticesCount();
    GAP *= 2;
    //dest_dir default
    if (dest_dir.empty())
    {
        //dg_file_path: xxx/human/data_graph/human.graph
        unsigned idx = dg_file_path.find_last_of("/");
        idx = dg_file_path.find_last_of("/", idx - 1);
        std::string dest_dir_par = dg_file_path.substr(0, idx);
        dest_dir = dest_dir_par + "/query_graph/";
    }
#if STEP_DEBUG == 1
    std::cout << "-d " << dg_file_path << std::endl;
    std::cout << "-v ";
    for (auto v : vcnt_arr)
    {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    std::cout << "-S ";
    for (auto v : sparse_arr)
    {
        std::cout << v << " ";
    }
    std::cout << std::endl;
    std::cout << "-j " << qcnt_required << std::endl;
    std::cout << "-o " << dest_dir << std::endl;
#endif //STEP_DEBUG
    gen_qgraph(dg, vcnt_arr, sparse_arr, qcnt_required, dest_dir, GAP);
    return 0;
}

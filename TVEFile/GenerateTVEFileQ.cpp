#include "../util/GenerateTVEFileQ.h"
#include "TVEFileG.h"

int main(int argc, char **argv)
{
    srand((unsigned)time(NULL));
    //-d dg_filename -v vcnt_arr -S sparse_arr -j qcnt_required
    std::string dg_filename; //"/media/data/hnu2022/yuanzhiqiu/word_test/data_graph/word_test.graph"; //xxx/data_graph/youtube.graph
    std::vector<unsigned> vcnt_arr = {8};
    std::vector<bool> sparse_arr = {0};
    unsigned qcnt_required = 200;
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
    TVEFileG *dg = new TVEFileG();
    dg->loadGraphFromFile(dg_filename);
    int GAP = dg->getVerticesCount();
    GAP *= 2;
    unsigned idx = dg_filename.find("data_graph");
    std::string dest_dir_par = dg_filename.substr(0, idx);
    std::string dest_dir = dest_dir_par + "query_graph/";
#ifdef DEBUG
    //std::cout << "dest_dir: " << dest_dir << std::endl;
#endif //DEBUG
    gen_qgraph(dg, vcnt_arr, sparse_arr, qcnt_required, dest_dir, GAP);
    return 0;
}

#include "TVEFileG.h"

/* WARNNING: 仅实现了离线计算topoMotif和labelMotif_limit的逻辑
*/
/* 写入的文件名prefix：dfile_prefix:"xxx/youtube"，将调用generateLabelMotifCount_limit或generateTopoMotifCount（需要传入"xxx/youtube"）
* data_graph已经加载基础结构
* structureKind
*/
void offlineCompute(MotifG *data_graph, std::string dfile_prefix, std::string structureKind)
{
    if (structureKind == "tmtf")
    {
#if TOPO_MOTIF_ENABLE == 1
        data_graph->generateTopoMotifCount(input_data_graph_file_prefix);
#else  //TOPO_MOTIF_ENABLE == 0:
        std::cout << "TOPO_MOTIF_ENABLE:" << TOPO_MOTIF_ENABLE << std::endl;
        std::cout << "TOPO_MOTIF_ENABLE == 1 is needed, modify config.h" << std::endl;
        return;
#endif //TOPO_MOTIF_ENABLE == 1
    }
    else if (structureKind == "lmtf_limit")
    {
#if LABEL_MOTIF_LIMIT == 1 && LABEL_MOTIF_ENABLE == 1
        data_graph->generateLabelMotifCount_limit(input_data_graph_file_prefix);
#else  //TOPO_MOTIF_ENABLE == 0 || LABEL_MOTIF_ENABLE==0:
        std::cout << "LABEL_MOTIF_LIMIT:" << LABEL_MOTIF_LIMIT << std::endl;
        std::cout << "LABEL_MOTIF_ENABLE:" << LABEL_MOTIF_ENABLE << std::endl;
        std::cout << "LABEL_MOTIF_LIMIT == 1 && LABEL_MOTIF_ENABLE == 1(the latter for nlf) is needed, modify config.h" << std::endl;
        return;
#endif //LABEL_MOTIF_LIMIT == 1 && LABEL_MOTIF_ENABLE==1
    }
}

/* 命令行参数：
* -d <data_graph_absolute_path> 
* -f <structureKind> (目前支持："tmtf", "lmtf_limit")
*/
int main(int argc, char **argv)
{
#if ONLINE_STAGE == 1
    std::cout << "ONLINE_STAGE:" << ONLINE_STAGE << std::endl;
    std::cout << "ONLINE_STAGE == 0 is needed, modify config.h" << std::endl;
    return 0;
#endif //#if ONLINE_STAGE==1
    std::set<std::string> structureKindset = {"tmtf", "lmtf_limit"};
    std::string input_data_graph_file, structureKind;

    if (argc != 5)
    {
        std::cout << "usage:" << std::endl;
        std::cout << "-d data_graph_absolute_path -f <structureKind>(";
        for (auto s : structureKindset)
        {
            std::cout << s << " ";
        }
        std::cout << ")" << std::endl;
        std::cout << "[example]: \n";
        std::cout << "./offline -d /media/data/hnu2022/yuanzhiqiu/human/data_graph/human.graph -f tmtf" << std::endl;
        return 0;
    }
    std::string op, val;
    std::map<std::string, std::string> cmd;
    for (int i = 1; i < argc; i += 2)
    {
        op = argv[i];
        val = argv[i + 1];
        cmd[op] = val;
    }
    input_data_graph_file = cmd["-d"];
    structureKind = cmd["-f"];

#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
    std::cout << "load basic structure..." << std::endl;
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
    data_graph->printGraphBasicDetail();
    std::cout << "-----" << std::endl;
#endif //#if STEP_DEBUG == 1

#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
    std::cout << "compute " << structureKind << " structure..." << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1

    size_t idx = input_data_graph_file.find_last_of(".");
    std::string input_data_graph_file_prefix = input_data_graph_file.substr(0, idx); //xxx/youtube
    offlineCompute(data_graph, input_data_graph_file_prefix, structureKind);

    return 0;
}

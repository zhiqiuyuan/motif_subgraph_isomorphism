#include "../util/GraphFeatures.h"
#include "TVEFileG.h"

void printUsage()
{
    std::cout << "usage:\n./<binname> -d <data_graph_absolute_path> " << std::endl;
}

/* WARNNING:暂时没有写记录特征到-o文件中的逻辑，而只是输出到标准输出
*/
/* 命令行参数：
* -d <data_graph_absolute_path> 
* -o <可选,output_fname>(默认:-d上一级目录/Gfeatures.txt)
*/
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printUsage();
        return 0;
    }
    std::string option, optionstr;
    std::map<std::string, std::string> cmd;
    for (int i = 1; i < argc; i += 2)
    {
        option = argv[i];
        optionstr = argv[i + 1];
        cmd[option] = optionstr;
    }
    std::string dfname = cmd["-d"]; //xxx/xxxdata_graph/youtube.graph
    std::string output_fname;
    if (cmd.count("-o"))
    {
        output_fname = cmd["-o"];
    }
    else
    {
        unsigned idx = dfname.find_last_of('/');
        idx = dfname.find_last_of('/', idx - 1);
        output_fname = dfname.substr(0, idx) + "/Gfeatures.txt";
    }

#if BASIC_RUNNING_COMMENT == 1 || RUNNING_COMMENT == 1
    std::cout << "load basic structure..." << std::endl;
#endif //#if BASIC_RUNNING_COMMENT==1 || RUNNING_COMMENT==1
    TVEFileG *data_graph = new TVEFileG();
    data_graph->loadGraphFromFile(dfname);
    GraphFeatures gf(data_graph);
    std::cout << gf.getTotalBidegree() << std::endl;
    delete data_graph;
}
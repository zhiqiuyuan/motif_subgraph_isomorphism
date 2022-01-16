#include "tools.h"
/* NOTICE: 此文件仅依赖tools.h中的Rand函数，不依赖项目中其他任何内容*/
/* EFile2TVEFile: 1.add numeric label 2.write as TVEFile format
* 1.add numeric label: randomly assigned a label out of 20 distinct labels to each vertex
* 没有新增一个EFile派生类是觉得EFile格式没有顶点标签，要加顶点标签的话本来就要处理数据图文件而且要加上V项，这样已经和TVEFile format很相似了；
* 另外CSR格式存储图（Graph的实现）需要在申请结构时已知一些信息，这些信息基本得处理完EFile才能得到，而已经写在TVEFile中可以直接读取
*/

void printUsage()
{
    std::cout << "usage:\n";
    std::cout << "-d <数据图文件绝对路径>" << std::endl;
    std::cout << "-ld <可选，label_distribute_type，目前支持取值uniform（均匀分布，随机给顶点加标签）>(默认：uniform)" << std::endl;
    std::cout << "-lt <可选，label_type，目前支持取值num（数值型，从0开始）>(默认：num)" << std::endl;
    std::cout << "-lsz <可选，label_set的大小>(默认：20)" << std::endl;
    std::cout << "-o <可选，输出数据图文件的绝对路径>(默认：-d的数据图文件同级目录同名文件，后缀名改成.graph)" << std::endl;
}

/* 命令行参数：
* -d <数据图文件绝对路径>
* -ld <可选,label_distribute_type,目前支持取值uniform（均匀分布，随机给顶点加标签）>(默认:uniform)
* -lt <可选,label_type,目前支持取值num（数值型，从0开始）>(默认:num)
* -lsz <可选,label_set 的大小>(默认:20)
* -o <可选，输出数据图文件的绝对路径>(默认：-d的数据图文件同级目录同名文件，后缀名改成.graph)
*/

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printUsage();
        return 0;
    }
    std::string dfname, label_distribute_type, label_type, output_dfname;
    unsigned label_set_sz = 20;
    label_distribute_type = "uniform";
    label_type = "num";

    std::string option, optionstr;
    std::map<std::string, std::string> cmd;
    for (int i = 1; i < argc; i += 2)
    {
        option = argv[i];
        optionstr = argv[i + 1];
        cmd[option] = optionstr;
    }
    dfname = cmd["-d"]; //xxx/youtube.txt
    if (cmd.count("-ld"))
    {
        label_distribute_type = cmd["-ld"];
    }
    if (cmd.count("-lt"))
    {
        label_type = cmd["-lt"];
    }
    if (cmd.count("-lsz"))
    {
        label_set_sz = std::stoi(cmd["-lsz"]);
    }
    if (cmd.count("-o"))
    {
        output_dfname = cmd["-o"];
    }
    else
    {
        //-d xxx/youtube.txt同级目录同名文件，后缀名改成.graph
        unsigned idx = dfname.find_last_of('.');
        output_dfname = dfname.substr(0, idx) + ".graph";
    }

    std::ifstream fin(dfname);
    if (fin.is_open() == 0)
    {
        std::cout << dfname << " not opened" << std::endl;
        return 0;
    }
    std::ofstream fout(output_dfname);
    if (fout.is_open() == 0)
    {
        std::cout << output_dfname << " not opened" << std::endl;
        return 0;
    }

    unsigned n, e;
    char t;
    fin >> t >> n >> e;
    fout << "t " << n << " " << e << "\n";
    //统计顶点度数
    unsigned *degree = new unsigned[n]();
    unsigned sid, tid;
    while (fin >> sid >> tid)
    {
        degree[sid]++;
        degree[tid]++;
    }
    /* WARNNING: WRONG METHOD:"先写边，然后fout回到文件首，写tne然后写顶点"：
    这样跳回文件首开始写顶点的话会把原来写好的东西给覆盖掉，这样逻辑不对
    fout.seekp(0, std::ios_base::beg);*/

    //vertex: v nodeid labelid degree
    if (label_distribute_type == "uniform" && label_type == "num")
    {
        for (unsigned i = 0; i < n; ++i)
        {
            fout << "v " << i << " " << Rand(label_set_sz) << " " << degree[i] << "\n";
        }
    }
    else
    {
        std::cout << "not supported [label_distribute_type:" << label_distribute_type << " label_type:" << label_type << "]" << std::endl;
    }

    //edge
    //fin回到文件首
    fin.close();
    fin.open(dfname);
    //fin.seekg(0, std::ios_base::beg);
    std::string line;
    std::getline(fin, line); //tne line
    while (fin >> sid >> tid)
    {
        fout << "e " << sid << " " << tid << "\n";
    }

    fin.close();
    fout.close();
    delete[] degree;
    return 0;
}
#include "MotifG.h"

MotifG::MotifG()
{
}

MotifG::~MotifG()
{
}

/*
 * LOAD MOTIF
 */
#if LABEL_MOTIF_ENABLE == 1
/* WARNNING:此函数注释掉的实现的逻辑为分配空间并从youtube_directed_label_kmethod_kInFileName中加载motif结构，此函数还没有实现
* filename_prefix: xxx/youtube,xxx/youtube_label.txt中加载motif_struct

此函数注释掉的部分实现的逻辑如下：
1.有向图 从文件中加载数据图的motif_count（此函数申请空间）
2.filename_prefix:"xxx/youtube",将从xxx/youtube_directed_label_kmethod_kInFileName.txt中加载motif_struct，每个顶点ko维
3.引用返回该图中最小最大lmotif种类数（over V(G)）
4.file format:
vertexID
code_arr(ascending code,k entries)
cnt_arr(corresponding to code_arr)

所有顶点后的一行：
max_lmtf_kind_cnt（G中lmotif最大种类数（over V(G)）） min_lmtf_kind_cnt
*/
void MotifG::loadLabelMotifCountFromFile(std::string filename_prefix)
{
    /*
    filename_prefix += "_directed_label_" + kmethod_name + "_" + std::to_string(kInFileName) + ".txt";
#ifdef DEBUG
    std::cout << "load motif count for data_graph from file:" << filename_prefix << std::endl;
#endif //DEBUG

    std::ifstream fin(filename_prefix);
    //allocate //G:不用存label_motif_count_sz_
    label_motif_count_ = new Lmtf *[vertices_count_];
    unsigned vid;
    std::string following_in_line;
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        label_motif_count_[i] = new Lmtf[k];
        fin >> vid;
        unsigned e = k;
        unsigned tmpcnt;
        if (kmethod_name == "mid")
        { // for code ascending in G_motif_arr
            unsigned mid = k / 2;
            for (unsigned j = 0; j <= mid; ++j)
            {
                if (mid - j >= 0) //left first
                {
                    fin >> label_motif_count_[i][mid - j].code;
                }
                if (j && mid + j < k) //right next
                {
                    fin >> label_motif_count_[i][mid + j].code;
                }
            }
            std::getline(fin, following_in_line); //absorb the rest of this line
            for (unsigned j = 0; j <= mid; ++j)
            {
                if (mid - j >= 0) //left first
                {
                    fin >> label_motif_count_[i][mid - j].cnt;
                }
                if (j && mid + j < k) //right next
                {
                    fin >> label_motif_count_[i][mid + j].cnt;
                }
            }
            std::getline(fin, following_in_line); //absorb the rest of this line
        }
        else
        {
            for (unsigned j = 0; j < k; ++j)
            {
                fin >> label_motif_count_[i][j].code;
            }
            std::getline(fin, following_in_line); //absorb the rest of this line
            for (unsigned j = 0; j < k; ++j)
            {
                fin >> tmpcnt;
                label_motif_count_[i][j].cnt = tmpcnt;
                if (tmpcnt == 0 && e == k)
                {
                    e = j;
                }
            }
            std::getline(fin, following_in_line); //absorb the rest of this line
        }
        if (kmethod_name == "top" || kmethod_name == "down")
        {
            //sort code_ascending
            std::sort(label_motif_count_[i], label_motif_count_[i] + e, code_ascending);
        }
    }
    //fin >> max_kind >> min_kind;
    fin.close();
    */
}

#if LABEL_MOTIF_LIMIT == 1
/* 从xxx/youtube_label_limit.txt中加载到std::map<unsigned, unsigned> *label_motif_map_
* filename_prefix:xxx/youtube
* 在此函数中给label_motif_map_分配空间
*/
/*
file format:
vid code_cnt
code cnt code cnt……
*/
void MotifG::loadLabelMotifCountFromFile_limit(std::string filename_prefix)
{
    label_motif_map_ = new std::map<unsigned, unsigned>[vertices_count_];
    unsigned vid, code_cnt, code, cnt;
    std::ifstream fin(filename_prefix + "_label_limit.txt");
    if (fin.is_open() == 0)
    {
        std::cout << filename_prefix << "_label_limit.txt not exit" << std::endl;
        return;
    }
    std::map<unsigned, unsigned> *one_vertex_cnt_map;
    while (fin >> vid >> code_cnt)
    {
        one_vertex_cnt_map = label_motif_map_ + vid;
        for (unsigned i = 0; i < code_cnt; ++i)
        {
            fin >> code >> cnt;
            (*one_vertex_cnt_map)[code] = cnt;
        }
    }
    fin.close();
}
#endif //LABEL_MOTIF_LIMIT
#endif //LABEL_MOTIF_ENABLE==1

#if TOPO_MOTIF_ENABLE == 1
/* filename_prefix:xxx/youtube，从xxx/youtube_topo.txt中加载到unsigned **motif_count_; 
file format: 
VertexID MOTIF_COUNT_DEMENSIONcount
*/
void MotifG::loadTopoMotifCountFromFile(std::string filename_prefix)
{
    filename_prefix += "_topo";
    std::ifstream fin(filename_prefix + ".txt");
    if (!fin.is_open())
    {
        std::cout << "file not exited: " << filename_prefix + ".txt" << std::endl;
        return;
    }

    motif_count_ = new unsigned *[vertices_count_];
    unsigned i;
    for (i = 0; i < vertices_count_; ++i)
    {
        motif_count_[i] = new unsigned[MOTIF_COUNT_DEMENSION];
    }
    unsigned *motif_cnt;
    std::string line;
    VertexID vid;
    while (getline(fin, line))
    {
        std::stringstream ss(line);
        ss >> vid;
        motif_cnt = motif_count_[vid];
        for (i = 0; i < MOTIF_COUNT_DEMENSION; ++i)
        {
            ss >> motif_cnt[i];
        }
    }

    fin.close();
}
#endif //TOPO_MOTIF_ENABLE==1

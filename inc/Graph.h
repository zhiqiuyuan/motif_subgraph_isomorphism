#ifndef SUBGRAPHMATCHING_GRAPH_H
#define SUBGRAPHMATCHING_GRAPH_H

#include <unordered_map>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "types.h"
#include "config.h"
#include <unistd.h>

/**
 * A graph is stored as the CSR format.
 */

class Graph
{
private:
    ui vertices_count_;
    ui edges_count_;
    ui labels_count_;
    ui max_degree_;
    ui max_label_frequency_;

    //vid->label
    LabelID *labels_;

    //label->all vids
    ui *reverse_index_offsets_;
    ui *reverse_index_;

    //label->frequency
    std::unordered_map<LabelID, ui> labels_frequency_;

    //vid->neighbors
    ui *offsets_;

    //neighbors,nlf structure
#if DIRECTED_GRAPH == 0
    VertexID *neighbors_;
#if OPTIMIZED_LABELED_GRAPH == 1
    //vid,label->neighbors
    ui *labels_offsets_; //labels_offsets_类似二维数组，[vid][label]索引（id为vid的顶点的一阶邻居中所有标签为label的顶点开始下标）
    std::unordered_map<LabelID, ui> *nlf_;
#endif
#else
    //vid->in neighbors
    ui *in_neighbors_nums_;
    VertexID *in_neighbors_;

    //vid->out neighbors
    ui *out_neighbors_nums_;
    VertexID *out_neighbors_;

    //vid->bi neighbors
    ui *bi_neighbors_nums_;
    VertexID *bi_neighbors_;
#if OPTIMIZED_LABELED_GRAPH == 1
    std::unordered_map<LabelID, ui> *in_nlf_;
    std::unordered_map<LabelID, ui> *out_nlf_;
    std::unordered_map<LabelID, ui> *bi_nlf_;
#endif
#endif // DIRECTED_GRAPH==0

    //motif structure
#if DIRECTED_GRAPH == 1
#if TOPO_MOTIF_ENABLE == 1
    ui **motif_count_; //|V|*24
#endif                 //TOPO_MOTIF_ENABLE==1
#if LABEL_MOTIF_ENABLE == 1
    //id为vid的顶点的motif_count_ 24维数组首地址为motif_count_[vid]
    Lmtf **label_motif_count_;
    //每个vertex一个Lmtf数组（G:每个Lmtf数组k维；q:每个Lmtf数组维度稀疏）
    ui *label_motif_count_sz_; //label_motif_count_数组每行有多少entries（对于q有用）
    ui k;                      //demesion of label_motif_count_ in data_graph is: k per vertex

#if LABEL_MOTIF_LIMIT == 1
    //每个顶点一个map:code->cnt
    std::map<ui, ui> *label_motif_map_;
#endif //LABEL_MOTIF_LIMIT==1

#endif //LABEL_MOTIF_ENABLE == 1
#else
#if TOPO_MOTIF_ENABLE == 1
    ui *tri_count_;
#endif //TOPO_MOTIF_ENABLE==1
#if LABEL_MOTIF_ENABLE == 1
    std::map<ui, std::map<ui, ui>> *label_tri_count_;
    //AB 2tri : A->B->2
    //记录时统一A<B再记录
#endif //LABEL_MOTIF_ENABLE==1
#endif // DIRECTED_GRAPH

private:
    void BuildReverseIndex();
#if OPTIMIZED_LABELED_GRAPH == 1
#if DIRECTED_GRAPH == 0
    void BuildNLF();
#else
    void BuildNLF_directed();
#endif
#endif

    /*============================
TOPO MOTIF OP
*/
#if TOPO_MOTIF_ENABLE == 1
public:
    //filename_prefix:xxx/youtube（没有.graph），结果写入xxx/youtube_directed.txt或者xxx/youtube_undirected.txt
    void generateMotifCount(std::string filename);
    //filename_prefix:"xxx/youtube",将从xxx/youtube_(un)directed.txt中加载motif_struct
    void loadMotifCountFromFile(std::string filename_prefix);

private:
#if DIRECTED_GRAPH == 1
    //for vertex i
    //value in motif_cnt needs to be already specified
    //memory it points to needs to be already allocated
    void directed_motif_count(ui i, ui *motif_cnt);
    //motif_detail:对于每种motif记录当初计数时是计算了哪两个顶点的
    void directed_motif_count_debug(ui i, ui *motif_cnt, std::vector<std::vector<std::pair<int, int>>> &motif_detail);
#else
    void undirected_motif_count(ui i, ui &cnt);
#endif //DIRECTED
#endif // TOPO_MOTIF_ENABLE

    /*============================
LABEL MOTIF OP
*/
#if LABEL_MOTIF_ENABLE == 1
public:
    //label_directed_online
    //label_undirected_online,label_undirected_offline
    //filename_prefix:"xxx/youtube"，将把motif写入到xxx/youtube_directed_label.txt中
    void generateMotifCount_label(std::string filename);
#if DIRECTED_GRAPH == 1 && ONLINE_STAGE == 0
    //label_directed_offline
    //filename_prefix:"xxx/youtube"，将把motif写入到xxx/youtube_directed_label_top_k.txt中
    //追加写
    //从vertexID为vertexID_begin的顶点开始数，每数vertex_period个顶点关闭打开一次文件（减少被断开然后结果无保留）
    void generateMotifCount_label(std::string filename, ui vertexID_begin, ui vertex_period, ui kb, ui ke, std::string *kmethod_set, ui kmethod_num, bool wrtMaxMinKind2file);
    //统计频率为1的特征占所有特征的占比
    void generateMotifCount_label(ui &one_cnt, ui &total_cnt);
#endif //DIRECTED_GRAPH == 1 && ONLINE_STAGE == 0 // &&LABEL_MOTIF_ENABLE == 1

#if DIRECTED_GRAPH == 1
#if LABEL_MOTIF_LIMIT == 1
    void generateMotifCount_label_limit(std::string filename_prefix);
    //filename_prefix:"xxx/youtube",xxx/youtube_directed_label_limit.txt中加载
    void loadMotifCountFromFile_label_limit(std::string filename_prefix);
#endif //LABEL_MOTIF_LIMIT==1
    //filename_prefix:"xxx/youtube",xxx/youtube_directed_label_kmethod_kInFileName.txt中加载motif_struct，每个顶点ko维
    void loadMotifCountFromFile_label(std::string filename_prefix, ui kInFileName, std::string kmethod_name, ui ko);
#else
    //filename_prefix:""xxx/youtube"，将从xxx/youtube_undirected_label.txt中加载motif_struct
    void loadMotifCountFromFile_label(std::string filename_prefix);
#endif //FIRECTED

private:
#if DIRECTED_GRAPH == 1
    //for vertex i
    //value in motif_cnt needs to be already specified
    //memory it points to needs to be already allocated
    void directed_label_motif_count(ui i, std::map<ui, ui> *cnt_map_arr);
    //impossible_code_entry_num:如果有mid方法，则通过此返回k-std::min(k, lmtf_arr_sz)，即对齐部分有多少entry，用于给mid先在有效部分选
    void choose_k_from_lmtf_arr(Lmtf *lmtf_arr, ui lmtf_arr_sz, ui k, Lmtf **lmtf_arr_G, std::string *kmethod_set, ui kmethod_num, int &impossible_code_entry_num);

#if LABEL_MOTIF_LIMIT == 1
    //for vertex i
    //value in one_vertex_cnt_map needs to be already specified
    //memory it points to needs to be already allocated
    void directed_label_motif_count_limit(ui i, std::map<ui, ui> *one_vertex_cnt_map);
    //还记录具体是哪两个顶点
    void directed_label_motif_count_limit_debug(ui i, std::map<ui, ui> *one_vertex_cnt_map, std::map<ui, std::vector<ui>> *code2vertexvertex);
#endif //LABEL_MOTIF_LIMIT==1
#else
    void undirected_label_motif_count(ui i, std::map<ui, std::map<ui, ui>> *label_tri_map);
#endif //DIRECTED
#endif //LABEL_MOTIF_ENABLE==1

    /*============================
BASIC OP
*/
public:
    //load
    Graph()
    {
        vertices_count_ = 0;
        edges_count_ = 0;
        labels_count_ = 0;
        max_degree_ = 0;
        max_label_frequency_ = 0;

        offsets_ = NULL;
        labels_ = NULL;
        reverse_index_offsets_ = NULL;
        reverse_index_ = NULL;
        labels_frequency_.clear();

/*
directed
*/
#if DIRECTED_GRAPH == 1
        in_neighbors_nums_ = NULL;
        in_neighbors_ = NULL;

        out_neighbors_nums_ = NULL;
        out_neighbors_ = NULL;

        bi_neighbors_nums_ = NULL;
        bi_neighbors_ = NULL;
#if TOPO_MOTIF_ENABLE == 1
        motif_count_ = NULL;
#endif //TOPO_MOTIF_ENABLE==1
#if LABEL_MOTIF_ENABLE == 1
        label_motif_count_ = NULL;
        label_motif_count_sz_ = NULL;
#if LABEL_MOTIF_LIMIT == 1
        label_motif_map_ = NULL;
#endif //LABEL_MOTIF_LIMIT==1
#endif //LABEL_MOTIF_ENABLE==1

#if OPTIMIZED_LABELED_GRAPH == 1
        in_nlf_ = NULL;
        out_nlf_ = NULL;
        bi_nlf_ = NULL;
#endif
/*
undirected
*/
#else
        neighbors_ = NULL;
#if TOPO_MOTIF_ENABLE == 1
        tri_count_ = NULL;
#endif //TOPO_MOTIF_ENABLE==1
#if LABEL_MOTIF_ENABLE == 1
        label_tri_count_ = NULL;
#endif //LABEL_MOTIF_ENABLE==1
#if OPTIMIZED_LABELED_GRAPH == 1
        labels_offsets_ = NULL;
        nlf_ = NULL;
#endif
#endif
    }

    ~Graph()
    {
        delete[] offsets_;
        delete[] labels_;
        delete[] reverse_index_offsets_;
        delete[] reverse_index_;
/*
directed
*/
#if DIRECTED_GRAPH == 1
        delete[] in_neighbors_nums_;
        delete[] in_neighbors_;
        delete[] out_neighbors_nums_;
        delete[] out_neighbors_;
        delete[] bi_neighbors_nums_;
        delete[] bi_neighbors_;
#if OPTIMIZED_LABELED_GRAPH == 1
        delete[] in_nlf_;
        delete[] out_nlf_;
        delete[] bi_nlf_;
#endif //OPTIMIZED_LABELED_GRAPH == 1
#if TOPO_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
        for (ui i = 0; i < vertices_count_; ++i)
        {
            delete[] motif_count_[i];
        }
        delete[] motif_count_;
#endif //TOPO_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
#if LABEL_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
#if LABEL_MOTIF_LIMIT == 0
        for (ui i = 0; i < vertices_count_; ++i)
        {
            delete[] label_motif_count_[i];
        }
        delete[] label_motif_count_;
        delete[] label_motif_count_sz_;
#else  //LABEL_MOTIF_LIMIT == 1:
        delete[] label_motif_map_;
#endif //LABEL_MOTIF_LIMIT == 0
#endif //LABEL_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1

/*
undirected
*/
#else
        delete[] neighbors_;
#if OPTIMIZED_LABELED_GRAPH == 1
        delete[] nlf_;
        delete[] labels_offsets_;
#endif
#if MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
        delete[] tri_count_;
#endif // MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
#if LABEL_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
        delete[] label_tri_count_;
#endif //LABEL_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
#endif
    }

    /*
    label print as char: labelID+'A'
    - label->vertices
    directed:
    - each vertex: in out bi neighbors;
    - each vertex: in out bi nlf_struct(in egonetwork:each label freq);
    */
    void print_graph_detail();
#if LABEL_MOTIF_ENABLE == 1
    //use_mtf_sz_cnt为1：标签有向图会使用label_motif_count_sz_数组
    /*
    label print as char: labelID+'A'
    code print as: topoMotifType_label_label

    directed label:
    use_mtf_sz_cnt 1:use label_motif_count_sz_[vid] as dime; 0:use k as
    - each vertex: label_motif_count_, with dime entries
    */
    void print_graph_mtf(bool use_mtf_sz_cnt);
#if LABEL_MOTIF_LIMIT == 1
    void print_label_motif_map();
#endif // #if LABEL_MOTIF_LIMIT == 1
#endif //LABEL_MOTIF_ENABLE==1

#if DIRECTED_GRAPH == 0
    //file_path:"xxx/name.graph"，将从xxx/name.graph中加载图结构
    void loadGraphFromFile(const std::string &file_path);
#else
    //file_path:"xxx/name.graph"，将从xxx/name.graph中加载图结构
    void loadGraphFromFile_directed(const std::string &file_path);

#endif // DIRECTED_GRAPH

    void printGraphMetaData();

    const ui getLabelsCount() const
    {
        return labels_count_;
    }

    const ui getVerticesCount() const
    {
        return vertices_count_;
    }

    const ui getEdgesCount() const
    {
        return edges_count_;
    }

    const ui getGraphMaxDegree() const
    {
        return max_degree_;
    }

    const ui getGraphMaxLabelFrequency() const
    {
        return max_label_frequency_;
    }

    const ui getVertexDegree(const VertexID id) const
    {
        return offsets_[id + 1] - offsets_[id];
    }

//get directed degree
#if DIRECTED_GRAPH == 1
    const ui getVertexInDegree(const VertexID id) const
    {
        return in_neighbors_nums_[id];
    }
    const ui getVertexOutDegree(const VertexID id) const
    {
        return out_neighbors_nums_[id];
    }
    const ui getVertexBiDegree(const VertexID id) const
    {
        return bi_neighbors_nums_[id];
    }
#endif // DIRECTED_GRAPH

    const ui getLabelsFrequency(const LabelID label) const
    {
        return labels_frequency_.find(label) == labels_frequency_.end() ? 0 : labels_frequency_.at(label);
    }

    const LabelID getVertexLabel(const VertexID id) const
    {
        return labels_[id];
    }
    const ui *getVerticesByLabel(const LabelID id, ui &count) const
    {
        count = reverse_index_offsets_[id + 1] - reverse_index_offsets_[id];
        return reverse_index_ + reverse_index_offsets_[id];
    }

//get neighbors
#if DIRECTED_GRAPH == 1
    const ui *getVertexInNeighbors(const VertexID id, ui &count) const
    {
        count = in_neighbors_nums_[id];
        return in_neighbors_ + offsets_[id];
    }
    const ui *getVertexOutNeighbors(const VertexID id, ui &count) const
    {
        count = out_neighbors_nums_[id];
        return out_neighbors_ + offsets_[id];
    }
    const ui *getVertexBiNeighbors(const VertexID id, ui &count) const
    {
        count = bi_neighbors_nums_[id];
        return bi_neighbors_ + offsets_[id];
    }
#else
    const ui *getVertexNeighbors(const VertexID id, ui &count) const
    {
        count = offsets_[id + 1] - offsets_[id];
        return neighbors_ + offsets_[id];
    }
#endif //DIRECTED_GRAPH

//get and release motif structure
#if DIRECTED_GRAPH == 0
#if TOPO_MOTIF_ENABLE == 1
    ui getVertexTriCount(const VertexID id) const
    {
        return tri_count_[id];
    }
#endif //TOPO_MOTIF_ENABLE==1
#if LABEL_MOTIF_ENABLE == 1
    std::map<ui, std::map<ui, ui>> *getVertexLabelTriCount(const VertexID id) const
    {
        return label_tri_count_ + id;
    }
#endif //LABEL_MOTIF_ENABLE==1
#else
#if TOPO_MOTIF_ENABLE == 1
    ui *getVertexMotifCount(const VertexID id) const
    {
        return motif_count_[id];
    }
#endif //TOPO_MOTIF_ENABLE==1
#if LABEL_MOTIF_ENABLE == 1
    Lmtf *getVertexLabelMotifCount(const VertexID id) const
    {
        return label_motif_count_[id];
    }
    Lmtf *getVertexLabelMotifCount(const VertexID id, ui &cnt) const
    {
        cnt = label_motif_count_sz_[id];
        return label_motif_count_[id];
    }
    const ui getK() const
    {
        return k;
    }
#if LABEL_MOTIF_LIMIT == 1
    std::map<ui, ui> *getVertexLabelMotifMap(const VertexID id) const
    {
        return label_motif_map_ + id;
    }
#endif //LABEL_MOTIF_LIMIT==1
#if ONLINE_STAGE == 1
    void alloc_label_motif_count()
    {
        label_motif_count_ = new Lmtf *[vertices_count_];
        for (ui i = 0; i < vertices_count_; ++i)
        {
            label_motif_count_[i] = new Lmtf[1];
        }
    }
    void delete_label_motif_count_()
    {
        delete[] label_motif_count_;
    }
#endif //ONLINE_STAGE == 1
#endif //LABEL_MOTIF_ENABLE==1
#endif //DIRECTED_GRAPH == 0

//get nlf
#if OPTIMIZED_LABELED_GRAPH == 1
#if DIRECTED_GRAPH == 1
    const std::unordered_map<LabelID, ui> *getVertexInNLF(const VertexID id) const
    {
        return in_nlf_ + id;
    }
    const std::unordered_map<LabelID, ui> *getVertexOutNLF(const VertexID id) const
    {
        return out_nlf_ + id;
    }
    const std::unordered_map<LabelID, ui> *getVertexBiNLF(const VertexID id) const
    {
        return bi_nlf_ + id;
    }
#else
    const ui *getNeighborsByLabel(const VertexID id, const LabelID label, ui &count) const
    {
        ui offset = id * labels_count_ + label; //OPTIMIZED_LABELED_GRAPH==1时，labels_offsets_类似二维数组，[vid][label]索引
        count = labels_offsets_[offset + 1] - labels_offsets_[offset];
        return neighbors_ + labels_offsets_[offset];
    }
    const std::unordered_map<LabelID, ui> *getVertexNLF(const VertexID id) const
    {
        return nlf_ + id;
    }

#endif // DIRECTED_GRAPH
#endif

//check edge //enhance: employee label
#if DIRECTED_GRAPH == 1
    //  u->v
    bool checkEdgeExistence_strict(VertexID u, VertexID v) const
    {
        ui v_in_count, cnt;
        const VertexID *neighbors = getVertexOutNeighbors(u, cnt);            //u out
        const VertexID *v_in_neighbors = getVertexInNeighbors(v, v_in_count); //v in
        if (cnt > v_in_count)
        { //find u in v_in
            cnt = v_in_count;
            neighbors = v_in_neighbors;
        }
        else
        { //find v in u_out
            u = v;
        }

        //find u in neighbors
        int begin = 0;
        int end = cnt - 1;
        while (begin <= end)
        {
            int mid = begin + ((end - begin) >> 1);
            if (neighbors[mid] == u)
            {
                return true;
            }
            else if (neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }

        return false;
    }
    //  u<->v
    bool checkEdgeExistence_bi(VertexID u, VertexID v) const
    {
        ui v_in_count, cnt;
        const VertexID *neighbors = getVertexBiNeighbors(u, cnt);             //u bi
        const VertexID *v_in_neighbors = getVertexBiNeighbors(v, v_in_count); //v bi
        if (cnt > v_in_count)
        { //find u in v_bi
            cnt = v_in_count;
            neighbors = v_in_neighbors;
        }
        else
        { //find v in u_bi
            u = v;
        }
        int begin = 0;
        int end = cnt - 1;
        while (begin <= end)
        {
            int mid = begin + ((end - begin) >> 1);
            if (neighbors[mid] == u)
            {
                return true;
            }
            else if (neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }

        return false;
    }
#else
    bool checkEdgeExistence(VertexID u, VertexID v) const
    {
        if (getVertexDegree(u) < getVertexDegree(v))
        {
            std::swap(u, v);
        }
        //degree u > degree v
        ui count = 0;
        const VertexID *neighbors = getVertexNeighbors(v, count);

        int begin = 0;
        int end = count - 1;
        while (begin <= end)
        {
            int mid = begin + ((end - begin) >> 1);
            if (neighbors[mid] == u)
            {
                return true;
            }
            else if (neighbors[mid] > u)
                end = mid - 1;
            else
                begin = mid + 1;
        }

        return false;
    }

#endif
};

#endif //SUBGRAPHMATCHING_GRAPH_H

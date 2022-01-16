#ifndef _GRAPH_H
#define _GRAPH_H

#include "../util/types.h"
#include "../util/GraphFeatures.h"
#include "../config/config.h"

/*
 * A graph is stored as the CSR format.
 */

class Graph
{
protected:
    /* NOTE: in out bi neighbors are not disjoint, in intersact with out is bi*/
    unsigned vertices_count_;
    unsigned edges_count_;
    unsigned labels_count_;
    unsigned max_label_frequency_;
    unsigned max_single_degree_; //所有顶点in out bi_degree中的最大值
    unsigned max_degree_;        //in+out

    //vid->label
    LabelID *labels_;

    int *core_table_;      //core_2：度数>=2的最大联通子图（度数：indegree+outdegree）
    unsigned core_length_; //core_2子图的顶点数目

    //vid->neighbors
    unsigned *offsets_;
    //vid->in neighbors
    unsigned *in_neighbors_nums_;
    VertexID *in_neighbors_;
    //vid->out neighbors
    unsigned *out_neighbors_nums_;
    VertexID *out_neighbors_;
    //vid->bi neighbors
    unsigned *bi_neighbors_nums_;
    VertexID *bi_neighbors_;

    //双边特征（无第三边）：出入双邻居组合
    std::unordered_map<LabelID, unsigned> *in_nlf_; //每个vertex一个map
    std::unordered_map<LabelID, unsigned> *out_nlf_;
    std::unordered_map<LabelID, unsigned> *bi_nlf_;

    //label->all vids
    unsigned *reverse_index_offsets_;
    unsigned *reverse_index_;
    //label->frequency
    std::unordered_map<LabelID, unsigned> labels_frequency_;

public:
    Graph();
    virtual ~Graph();

    //file_path:"xxx/name.graph"
    virtual void loadGraphFromFile(const std::string &file_path) = 0;

    void buildCoreTable();
    //build nlf
    void BuildNLF();
    void BuildReverseIndex();

    const unsigned getLabelsCount() const
    {
        return labels_count_;
    }

    const unsigned getVerticesCount() const
    {
        return vertices_count_;
    }

    const unsigned getEdgesCount() const
    {
        return edges_count_;
    }

    const unsigned getVertexInDegree(const VertexID id) const
    {
        return in_neighbors_nums_[id];
    }
    const unsigned getVertexOutDegree(const VertexID id) const
    {
        return out_neighbors_nums_[id];
    }
    const unsigned getVertexBiDegree(const VertexID id) const
    {
        return bi_neighbors_nums_[id];
    }
    const unsigned getVertexDegree(const VertexID id) const
    {
        return in_neighbors_nums_[id] + out_neighbors_nums_[id];
    }
    const unsigned getGraphMaxSingleDegree() const
    {
        return max_single_degree_;
    }
    const unsigned getGraphMaxDegree() const
    {
        return max_degree_;
    }

    const LabelID getVertexLabel(const VertexID id) const
    {
        return labels_[id];
    }
    const unsigned getGraphMaxLabelFrequency() const
    {
        return max_label_frequency_;
    }
    const unsigned *getVerticesByLabel(const LabelID id, unsigned &count) const
    {
        count = reverse_index_offsets_[id + 1] - reverse_index_offsets_[id];
        return reverse_index_ + reverse_index_offsets_[id];
    }
    const unsigned getLabelsFrequency(const LabelID label) const
    {
        return labels_frequency_.find(label) == labels_frequency_.end() ? 0 : labels_frequency_.at(label);
    }

    const unsigned getCoreValue(const VertexID id) const
    {
        return core_table_[id];
    }
    const unsigned get2CoreSize() const
    {
        return core_length_;
    }

    const unsigned *getVertexInNeighbors(const VertexID id, unsigned &count) const
    {
        count = in_neighbors_nums_[id];
        return in_neighbors_ + offsets_[id];
    }
    const unsigned *getVertexOutNeighbors(const VertexID id, unsigned &count) const
    {
        count = out_neighbors_nums_[id];
        return out_neighbors_ + offsets_[id];
    }
    const unsigned *getVertexBiNeighbors(const VertexID id, unsigned &count) const
    {
        count = bi_neighbors_nums_[id];
        return bi_neighbors_ + offsets_[id];
    }
    const std::unordered_map<LabelID, unsigned> *getVertexInNLF(const VertexID id) const
    {
        return in_nlf_ + id;
    }
    const std::unordered_map<LabelID, unsigned> *getVertexOutNLF(const VertexID id) const
    {
        return out_nlf_ + id;
    }
    const std::unordered_map<LabelID, unsigned> *getVertexBiNLF(const VertexID id) const
    {
        return bi_nlf_ + id;
    }

    //check edge //enhance: employee label
    //  u->v
    bool checkEdgeExistence(VertexID u, VertexID v) const;
    //  u<->v
    bool checkEdgeExistence_bi(VertexID u, VertexID v) const;

    /*
 * PRINT
 */
    virtual void printGraphMetaData();
    /*
    print_nlf==0:
    - each vertex: label; in out bi neighbors;
    (label print as char: labelID+'A')
    
    print_nlf==1:
    - each vertex: 
        label; (label print as char: labelID+'A')
        in out bi neighbors;
        in out bi nlf_struct(in egonetwork:each label freq); 
    */
    virtual void printGraphDetail(bool print_nlf);
};

#endif //_GRAPH_H

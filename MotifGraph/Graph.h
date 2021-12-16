#ifndef _GRAPH_H
#define _GRAPH_H

#include "../util/types.h"
#include "../config/config.h"

/*
 * A graph is stored as the CSR format.
 */

class Graph
{
protected:
    unsigned vertices_count_;
    unsigned edges_count_;
    unsigned labels_count_;
    unsigned max_label_frequency_;

    //vid->label
    LabelID *labels_;

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

public:
    Graph();
    virtual ~Graph();

    //file_path:"xxx/name.graph"
    virtual void loadGraphFromFile(const std::string &file_path) = 0;

    virtual void printGraphMetaData();
    /*
    - each vertex: label; in out bi neighbors;
    (label print as char: labelID+'A')
    */
    virtual void printGraphDetail();

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

    const LabelID getVertexLabel(const VertexID id) const
    {
        return labels_[id];
    }
    const unsigned getGraphMaxLabelFrequency() const
    {
        return max_label_frequency_;
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

    //check edge //enhance: employee label
    //  u->v
    bool checkEdgeExistence(VertexID u, VertexID v) const
    {
        unsigned v_in_count, cnt;
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
        unsigned v_in_count, cnt;
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
};

#endif //_GRAPH_H

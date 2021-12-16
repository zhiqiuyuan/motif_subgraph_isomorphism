#include "Graph.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string.h>

Graph::Graph()
{
    vertices_count_ = 0;
    edges_count_ = 0;
    labels_count_ = 0;
    max_label_frequency_ = 0;
    labels_ = NULL;

    offsets_ = NULL;
    in_neighbors_nums_ = NULL;
    in_neighbors_ = NULL;
    out_neighbors_nums_ = NULL;
    out_neighbors_ = NULL;
    bi_neighbors_nums_ = NULL;
    bi_neighbors_ = NULL;
}

Graph::~Graph()
{
    delete[] offsets_;
    delete[] labels_;

    delete[] in_neighbors_nums_;
    delete[] in_neighbors_;
    delete[] out_neighbors_nums_;
    delete[] out_neighbors_;
    delete[] bi_neighbors_nums_;
    delete[] bi_neighbors_;
}

void Graph::printGraphMetaData()
{
    std::cout << "|V|: " << vertices_count_ << ", |E|: " << edges_count_ << ", |label_set|: " << labels_count_ << std::endl;
}

/*
- each vertex: label; in out bi neighbors;
(label print as char: labelID+'A')
*/
void Graph::printGraphDetail()
{
    std::cout << "-----" << std::endl;
    std::cout << std::endl;
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        std::cout << "v" << i + 1 << ":" << char(getVertexLabel(i) + 'A') << std::endl;
        std::cout << "in neighbors: ";
        unsigned cnt;
        const unsigned *arr = getVertexInNeighbors(i, cnt);
        for (unsigned j = 0; j < cnt; ++j)
        {
            std::cout << "v" << arr[j] + 1 << " ";
        }
        std::cout << std::endl;

        std::cout << "out neighbors: ";
        arr = getVertexOutNeighbors(i, cnt);
        for (unsigned j = 0; j < cnt; ++j)
        {
            std::cout << "v" << arr[j] + 1 << " ";
        }
        std::cout << std::endl;

        std::cout << "bi neighbors: ";
        arr = getVertexBiNeighbors(i, cnt);
        for (unsigned j = 0; j < cnt; ++j)
        {
            std::cout << "v" << arr[j] + 1 << " ";
        }
        std::cout << "-----" << std::endl;
        std::cout << std::endl;
    }
}

//  u->v
bool Graph::checkEdgeExistence(VertexID u, VertexID v) const
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
bool Graph::checkEdgeExistence_bi(VertexID u, VertexID v) const
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

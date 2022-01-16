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
    max_single_degree_ = 0;
    max_degree_ = 0;
    labels_ = NULL;
    core_table_ = NULL;
    core_length_ = 0;

    offsets_ = NULL;
    in_neighbors_nums_ = NULL;
    in_neighbors_ = NULL;
    out_neighbors_nums_ = NULL;
    out_neighbors_ = NULL;
    bi_neighbors_nums_ = NULL;
    bi_neighbors_ = NULL;

    in_nlf_ = NULL;
    out_nlf_ = NULL;
    bi_nlf_ = NULL;
    reverse_index_offsets_ = NULL;
    reverse_index_ = NULL;
    labels_frequency_.clear();
}

Graph::~Graph()
{
    delete[] offsets_;
    delete[] labels_;
    if (core_table_ != NULL)
    {
        delete[] core_table_;
    }

    delete[] in_neighbors_nums_;
    delete[] in_neighbors_;
    delete[] out_neighbors_nums_;
    delete[] out_neighbors_;
    delete[] bi_neighbors_nums_;
    delete[] bi_neighbors_;
    if (in_nlf_ != NULL)
    {
        delete[] in_nlf_;
    }
    if (out_nlf_ != NULL)
    {
        delete[] out_nlf_;
    }
    if (bi_nlf_ != NULL)
    {
        delete[] bi_nlf_;
    }

    delete[] reverse_index_offsets_;
    delete[] reverse_index_;
}

void Graph::printGraphMetaData()
{
    std::cout << "|V|: " << vertices_count_ << ", |E|: " << edges_count_ << ", |label_set|: " << labels_count_ << std::endl;
}

void Graph::printGraphDetail(bool print_nlf)
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
        std::cout << std::endl;

        if (print_nlf)
        {
            std::cout << "in nlf: " << std::endl;
            const std::unordered_map<LabelID, unsigned> *pmap = getVertexInNLF(i);
            for (auto it : *pmap)
            {
                std::cout << char(it.first + 'A') << ":" << it.second << " ";
            }
            std::cout << std::endl;
            std::cout << "out nlf: " << std::endl;
            pmap = getVertexOutNLF(i);
            for (auto it : *pmap)
            {
                std::cout << char(it.first + 'A') << ":" << it.second << " ";
            }
            std::cout << std::endl;
            std::cout << "bi nlf: " << std::endl;
            pmap = getVertexBiNLF(i);
            for (auto it : *pmap)
            {
                std::cout << char(it.first + 'A') << ":" << it.second << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "-----" << std::endl;
    }
}

void Graph::buildCoreTable()
{
    core_table_ = new int[vertices_count_];
    GraphFeatures::getKCore(this, core_table_);

    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        if (core_table_[i] > 1) //i是core vertices
        {
            core_length_ += 1;
        }
    }
}

void Graph::BuildNLF()
{
    in_nlf_ = new std::unordered_map<LabelID, unsigned>[vertices_count_];
    out_nlf_ = new std::unordered_map<LabelID, unsigned>[vertices_count_];
    bi_nlf_ = new std::unordered_map<LabelID, unsigned>[vertices_count_];
    const VertexID *neighbors;
    unsigned cnt;
    LabelID label;
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        //count 3 kind
        neighbors = getVertexInNeighbors(i, cnt);
        for (unsigned j = 0; j < cnt; ++j)
        {
            label = getVertexLabel(neighbors[j]);
            if (in_nlf_[i].count(label) == 0)
            {
                in_nlf_[i][label] = 0;
            }
            in_nlf_[i][label]++;
        }
        neighbors = getVertexOutNeighbors(i, cnt);
        for (unsigned j = 0; j < cnt; ++j)
        {
            label = getVertexLabel(neighbors[j]);
            if (out_nlf_[i].count(label) == 0)
            {
                out_nlf_[i][label] = 0;
            }
            out_nlf_[i][label]++;
        }
        neighbors = getVertexBiNeighbors(i, cnt);
        for (unsigned j = 0; j < cnt; ++j)
        {
            label = getVertexLabel(neighbors[j]);
            if (bi_nlf_[i].count(label) == 0)
            {
                bi_nlf_[i][label] = 0;
            }
            bi_nlf_[i][label]++;
        }
    }
}

/*
 * LDF, NLF
 */
void Graph::BuildReverseIndex()
{
    reverse_index_ = new unsigned[vertices_count_];
    reverse_index_offsets_ = new unsigned[labels_count_ + 1];
    reverse_index_offsets_[0] = 0;

    unsigned total = 0;
    for (unsigned i = 0; i < labels_count_; ++i)
    {
        reverse_index_offsets_[i + 1] = total;
        total += labels_frequency_[i];
    }

    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        LabelID label = labels_[i];
        reverse_index_[reverse_index_offsets_[label + 1]++] = i;
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

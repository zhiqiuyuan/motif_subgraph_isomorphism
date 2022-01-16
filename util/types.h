#ifndef _TYPES_H
#define _TYPES_H

#include <cstdint>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <queue>

#include <algorithm>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <string.h>
#include <time.h>
#include <memory.h>

#include <unistd.h>

typedef unsigned VertexID;
typedef unsigned LabelID;

struct Lmtf
{
    unsigned code;
    unsigned cnt;
};
bool code_ascending(Lmtf l, Lmtf r);
bool cnt_descending(Lmtf l, Lmtf r);

class TreeNode
{
public:
    VertexID id_;
    VertexID parent_;
    unsigned level_;

    unsigned in_under_level_count_;
    unsigned out_under_level_count_;
    unsigned bi_under_level_count_;

    unsigned in_children_count_;
    unsigned out_children_count_;
    unsigned bi_children_count_;

    unsigned in_bn_count_;
    unsigned out_bn_count_;
    unsigned bi_bn_count_;

    unsigned in_fn_count_;
    unsigned out_fn_count_;
    unsigned bi_fn_count_;

    VertexID *in_under_level_;
    VertexID *out_under_level_;
    VertexID *bi_under_level_;

    VertexID *in_children_;
    VertexID *out_children_;
    VertexID *bi_children_;

    VertexID *in_bn_;
    VertexID *out_bn_;
    VertexID *bi_bn_;

    VertexID *in_fn_;
    VertexID *out_fn_;
    VertexID *bi_fn_;

    size_t estimated_embeddings_num_;

public:
    TreeNode()
    {
        id_ = 0;
        parent_ = 0;
        level_ = 0;
        estimated_embeddings_num_ = 0;

        in_under_level_ = NULL;
        out_under_level_ = NULL;
        bi_under_level_ = NULL;

        in_bn_ = NULL;
        out_bn_ = NULL;
        bi_bn_ = NULL;

        in_fn_ = NULL;
        out_fn_ = NULL;
        bi_fn_ = NULL;

        in_children_ = NULL;
        out_children_ = NULL;
        bi_children_ = NULL;

        in_under_level_count_ = 0;
        out_under_level_count_ = 0;
        bi_under_level_count_ = 0;

        in_children_count_ = 0;
        out_children_count_ = 0;
        bi_children_count_ = 0;

        in_bn_count_ = 0;
        out_bn_count_ = 0;
        bi_bn_count_ = 0;

        in_fn_count_ = 0;
        out_fn_count_ = 0;
        bi_fn_count_ = 0;
    }

    ~TreeNode()
    {
        delete[] in_under_level_;
        delete[] out_under_level_;
        delete[] bi_under_level_;

        delete[] in_bn_;
        delete[] out_bn_;
        delete[] bi_bn_;

        delete[] in_fn_;
        delete[] out_fn_;
        delete[] bi_fn_;

        delete[] in_children_;
        delete[] out_children_;
        delete[] bi_children_;
    }

    void initialize(const unsigned size)
    {
        in_under_level_ = new VertexID[size];
        out_under_level_ = new VertexID[size];
        bi_under_level_ = new VertexID[size];

        in_bn_ = new VertexID[size];
        out_bn_ = new VertexID[size];
        bi_bn_ = new VertexID[size];

        in_fn_ = new VertexID[size];
        out_fn_ = new VertexID[size];
        bi_fn_ = new VertexID[size];

        in_children_ = new VertexID[size];
        out_children_ = new VertexID[size];
        bi_children_ = new VertexID[size];
    }

    void clearCount()
    {
        in_under_level_count_ = 0;
        out_under_level_count_ = 0;
        bi_under_level_count_ = 0;

        in_children_count_ = 0;
        out_children_count_ = 0;
        bi_children_count_ = 0;

        in_bn_count_ = 0;
        out_bn_count_ = 0;
        bi_bn_count_ = 0;

        in_fn_count_ = 0;
        out_fn_count_ = 0;
        bi_fn_count_ = 0;
    }
};

#endif //_TYPES_H

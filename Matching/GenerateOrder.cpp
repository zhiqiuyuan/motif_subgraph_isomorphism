#include "GenerateOrder.h"

GenerateOrder::GenerateOrder()
{
}

GenerateOrder::~GenerateOrder()
{
}

//BFS树，order，level
//level_offset是在order上的（order是BFS出来的则同level的一定会group到一起）
void GenerateOrder::generateCFLFilterOrder(const Graph *data_graph, const Graph *query_graph, TreeNode *&tree,
                                           VertexID *&order, int &level_count, unsigned *&level_offset)
{
    unsigned query_vertices_num = query_graph->getVerticesCount();
    VertexID start_vertex = selectCFLFilterStartVertex(data_graph, query_graph);
    GraphFeatures::weakBFS(query_graph, start_vertex, tree, order);

    std::vector<unsigned> order_index(query_vertices_num);
    for (unsigned i = 0; i < query_vertices_num; ++i)
    {
        VertexID query_vertex = order[i];
        order_index[query_vertex] = i; //vertex idx on order array
    }

    level_count = -1;
    level_offset = new unsigned[query_vertices_num + 1];

    //按order
    for (unsigned i = 0; i < query_vertices_num; ++i)
    {
        VertexID u = order[i];
        /*
        tree[u].under_level_count_ = 0; //bfs树中u的孩子
        tree[u].bn_count_ = 0;          //前驱（order中u之前的顶点（中和u之间有边的））
        tree[u].fn_count_ = 0;          //后继（order中u之后的顶点（中和u之间有边的，去除孩子））
        */
        tree[u].clearCount();

        if (tree[u].level_ != level_count)
        { //新的一层
            level_count += 1;
            level_offset[level_count] = 0; //.level_==level_count的组
        }
        //level_count指向当前层

        level_offset[level_count] += 1;

        unsigned u_nbrs_count;
        const VertexID *u_nbrs;

        //in
        u_nbrs = query_graph->getVertexInNeighbors(u, u_nbrs_count);
        for (unsigned j = 0; j < u_nbrs_count; ++j)
        {
            VertexID u_nbr = u_nbrs[j];

            if (tree[u].level_ == tree[u_nbr].level_) //同层
            {
                if (order_index[u_nbr] < order_index[u])
                {
                    tree[u].in_bn_[tree[u].in_bn_count_++] = u_nbr; //u的前驱
                }
                else
                {
                    tree[u].in_fn_[tree[u].in_fn_count_++] = u_nbr; //u的后继
                }
            }
            else if (tree[u].level_ > tree[u_nbr].level_)
            {
                tree[u].in_bn_[tree[u].in_bn_count_++] = u_nbr; //u的前驱
            }
            else //tree[u].level_ < tree[u_nbr].level_: u_nbr在u下面的层中
            {
                tree[u].in_under_level_[tree[u].in_under_level_count_++] = u_nbr;
            }
        }

        //out
        u_nbrs = query_graph->getVertexOutNeighbors(u, u_nbrs_count);
        for (unsigned j = 0; j < u_nbrs_count; ++j)
        {
            VertexID u_nbr = u_nbrs[j];

            if (tree[u].level_ == tree[u_nbr].level_) //同层
            {
                if (order_index[u_nbr] < order_index[u])
                {
                    tree[u].out_bn_[tree[u].out_bn_count_++] = u_nbr; //u的前驱
                }
                else
                {
                    tree[u].out_fn_[tree[u].out_fn_count_++] = u_nbr; //u的后继
                }
            }
            else if (tree[u].level_ > tree[u_nbr].level_)
            {
                tree[u].out_bn_[tree[u].out_bn_count_++] = u_nbr; //u的前驱
            }
            else //tree[u].level_ < tree[u_nbr].level_: u_nbr在u下面的层中
            {
                tree[u].out_under_level_[tree[u].out_under_level_count_++] = u_nbr;
            }
        }

        //bi
        u_nbrs = query_graph->getVertexBiNeighbors(u, u_nbrs_count);
        for (unsigned j = 0; j < u_nbrs_count; ++j)
        {
            VertexID u_nbr = u_nbrs[j];

            if (tree[u].level_ == tree[u_nbr].level_) //同层
            {
                if (order_index[u_nbr] < order_index[u])
                {
                    tree[u].bi_bn_[tree[u].bi_bn_count_++] = u_nbr; //u的前驱
                }
                else
                {
                    tree[u].bi_fn_[tree[u].bi_fn_count_++] = u_nbr; //u的后继
                }
            }
            else if (tree[u].level_ > tree[u_nbr].level_)
            {
                tree[u].bi_bn_[tree[u].bi_bn_count_++] = u_nbr; //u的前驱
            }
            else //tree[u].level_ < tree[u_nbr].level_: u_nbr在u下面的层中
            {
                tree[u].bi_under_level_[tree[u].bi_under_level_count_++] = u_nbr;
            }
        }
    }
    //此时level_count指向最后一层，是最后一层的下标（下标从0开始），所以再+1则是总层数
    level_count += 1;

    unsigned prev_value = 0;
    for (unsigned i = 1; i <= level_count; ++i)
    {
        unsigned temp = level_offset[i];
        level_offset[i] = level_offset[i - 1] + prev_value;
        prev_value = temp;
    }
    level_offset[0] = 0;
}

VertexID GenerateOrder::selectCFLFilterStartVertex(const Graph *data_graph, const Graph *query_graph)
{
    auto rank_compare = [](std::pair<VertexID, double> l, std::pair<VertexID, double> r)
    {
        return l.second < r.second;
    };

    std::priority_queue<std::pair<VertexID, double>, std::vector<std::pair<VertexID, double>>, decltype(rank_compare)> rank_queue(rank_compare);

    // Compute the ranking.
    // double rank = frequency / (double)degree;
    for (unsigned i = 0; i < query_graph->getVerticesCount(); ++i)
    {
        VertexID query_vertex = i;
        //从core vertices中选开始顶点，或者如果没有core vertices则从query_graph所有顶点中选
        if (query_graph->get2CoreSize() == 0 || query_graph->getCoreValue(query_vertex) > 1)
        {
            LabelID label = query_graph->getVertexLabel(query_vertex);
            unsigned degree = query_graph->getVertexDegree(query_vertex);
            unsigned frequency = data_graph->getLabelsFrequency(label);
            double rank = frequency / (double)degree;
            rank_queue.push(std::make_pair(query_vertex, rank));
        }
    }

    // Keep the top-3.
    while (rank_queue.size() > 3)
    {
        rank_queue.pop();
    }

    VertexID start_vertex = 0;
    double min_score = data_graph->getGraphMaxLabelFrequency() + 1;

    while (!rank_queue.empty())
    {
        VertexID query_vertex = rank_queue.top().first;
        unsigned count;
        Filter::computeCandidateWithNLF(data_graph, query_graph, query_vertex, count);
        //count: |C(query_vertex)| after NLF
        double cur_score = count / (double)(query_graph->getVertexDegree(query_vertex));

        if (cur_score < min_score)
        {
            start_vertex = query_vertex;
            min_score = cur_score;
        }
        rank_queue.pop();
    }

    return start_vertex;
}

void GenerateOrder::generateDPisoFilterOrder(const Graph *data_graph, const Graph *query_graph, TreeNode *&tree,
                                             VertexID *&order)
{
    VertexID start_vertex = selectDPisoFilterStartVertex(data_graph, query_graph);
#if STEP_DEBUG == 1
    std::cout << "start_vertex:" << start_vertex << std::endl;
#endif //STEP_DEBUG
    GraphFeatures::weakBFS(query_graph, start_vertex, tree, order);

    unsigned query_vertices_num = query_graph->getVerticesCount();
    std::vector<unsigned> order_index(query_vertices_num);
    for (unsigned i = 0; i < query_vertices_num; ++i)
    {
        VertexID query_vertex = order[i];
        order_index[query_vertex] = i;
    }

    for (unsigned i = 0; i < query_vertices_num; ++i)
    {
        VertexID u = order[i];
        tree[u].clearCount();

        unsigned u_nbrs_count;
        const VertexID *u_nbrs;
        //in
        u_nbrs = query_graph->getVertexInNeighbors(u, u_nbrs_count);
        for (unsigned j = 0; j < u_nbrs_count; ++j)
        {
            VertexID u_nbr = u_nbrs[j];
            if (order_index[u_nbr] < order_index[u])
            {
                tree[u].in_bn_[tree[u].in_bn_count_++] = u_nbr;
            }
            else
            {
                tree[u].in_fn_[tree[u].in_fn_count_++] = u_nbr;
            }
        }
        //out
        u_nbrs = query_graph->getVertexOutNeighbors(u, u_nbrs_count);
        for (unsigned j = 0; j < u_nbrs_count; ++j)
        {
            VertexID u_nbr = u_nbrs[j];
            if (order_index[u_nbr] < order_index[u])
            {
                tree[u].out_bn_[tree[u].out_bn_count_++] = u_nbr;
            }
            else
            {
                tree[u].out_fn_[tree[u].out_fn_count_++] = u_nbr;
            }
        }
        //bi
        u_nbrs = query_graph->getVertexBiNeighbors(u, u_nbrs_count);
        for (unsigned j = 0; j < u_nbrs_count; ++j)
        {
            VertexID u_nbr = u_nbrs[j];
            if (order_index[u_nbr] < order_index[u])
            {
                tree[u].bi_bn_[tree[u].bi_bn_count_++] = u_nbr;
            }
            else
            {
                tree[u].bi_fn_[tree[u].bi_fn_count_++] = u_nbr;
            }
        }
    }
}

VertexID GenerateOrder::selectDPisoFilterStartVertex(const Graph *data_graph, const Graph *query_graph)
{
    double min_score = data_graph->getVerticesCount();
    VertexID start_vertex = 0;

    for (unsigned i = 0; i < query_graph->getVerticesCount(); ++i)
    {
        unsigned degree = query_graph->getVertexDegree(i);
        if (degree <= 1)
            continue;

        unsigned count = 0;
        Filter::computeCandidateWithLDF(data_graph, query_graph, i, count);
        double cur_score = count / (double)degree;
        if (cur_score < min_score)
        {
            min_score = cur_score;
            start_vertex = i;
        }
    }

    return start_vertex;
}

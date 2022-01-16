#include "GraphFeatures.h"

GraphFeatures::GraphFeatures(Graph *g) : graph(g)
{
}

GraphFeatures::~GraphFeatures()
{
}

//输出图中的bi degree之和
unsigned GraphFeatures::getTotalBidegree()
{
    unsigned vertices_cnt = graph->getVerticesCount();
    unsigned total_bidegree = 0;
    for (unsigned i = 0; i < vertices_cnt; ++i)
    {
        total_bidegree += graph->getVertexBiDegree(i);
    }
    return total_bidegree;
}

/*标签有向图：统计所有q图中频率为1的特征占比
#if LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH == 1
int main_1(int argc, char **argv)
{
    std::string head = "/media/data/hnu2022/yuanzhiqiu/";
    std::vector<std::string> g1 = {"dblp", "eu2005", "hprd", "patents", "yeast", "youtube"};
    std::vector<std::string> g2 = {"human", "wordnet"};
    std::vector<std::string> sd = {"sparse_", "dense_"};
    std::vector<unsigned> qset1 = {8, 16, 24, 32};
    std::vector<unsigned> qset2 = {8, 12, 16, 20};
    unsigned jb = 1, je = 200;
    unsigned one_cnt_ = 0, total_cnt_ = 0;
    for (auto g : g1)
    {
        unsigned one_cnt = 0, total_cnt = 0;
        for (auto s : sd)
        {
            std::string fname = head + g + "/query_graph/query_" + s; //xxx/query_sparse_
            for (auto i : qset1)
            {
                std::string fname1 = fname + std::to_string(i) + "_"; //xxx/query_sparse_4_
                for (unsigned j = jb; j <= je; ++j)
                {
                    std::string fname2 = fname1 + std::to_string(j) + ".graph"; //xxx/query_sparse_4_j.graph
                    //std::cout << fname2 << std::endl;
                    Graph *query_graph = new Graph();
                    query_graph->loadGraphFromFile_directed(fname2);
                    query_graph->generateMotifCount_label(one_cnt, total_cnt);
                    one_cnt_ += one_cnt;
                    total_cnt_ += total_cnt;
                    delete query_graph;
                }
            }
        }
        std::cout << g << ":\n";
        std::cout << "one_cne:" << one_cnt << "\ntotal_cnt:" << total_cnt << std::endl;
        std::cout << "one_cnt/total_cnt:" << (double(one_cnt)) / total_cnt << "\n"
                  << std::endl;
    }
    for (auto g : g2)
    {
        unsigned one_cnt = 0, total_cnt = 0;
        for (auto s : sd)
        {
            std::string fname = head + g + "/query_graph/query_" + s; //xxx/query_sparse_
            for (auto i : qset2)
            {
                std::string fname1 = fname + std::to_string(i) + "_"; //xxx/query_sparse_4_
                for (unsigned j = jb; j <= je; ++j)
                {
                    std::string fname2 = fname1 + std::to_string(j) + ".graph"; //xxx/query_sparse_4_j.graph
                    //std::cout << fname2 << std::endl;
                    Graph *query_graph = new Graph();
                    query_graph->loadGraphFromFile_directed(fname2);
                    query_graph->generateMotifCount_label(one_cnt, total_cnt);
                    one_cnt_ += one_cnt;
                    total_cnt_ += total_cnt;
                    delete query_graph;
                }
            }
        }
        std::cout << g << ":\n";
        std::cout << "one_cne:" << one_cnt << "\ntotal_cnt:" << total_cnt << std::endl;
        std::cout << "one_cnt/total_cnt:" << (double(one_cnt)) / total_cnt << "\n"
                  << std::endl;
    }

    std::cout << "one_cne_:" << one_cnt_ << "\ntotal_cnt_:" << total_cnt_ << std::endl;
    std::cout << "one_cnt/total_cnt:" << (double(one_cnt_)) / total_cnt_ << std::endl;
    return 0;
}
#endif //#if LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH==1

*/

void GraphFeatures::weakBFS(VertexID root_vertex, TreeNode *&tree, VertexID *&bfs_order)
{
    unsigned vertex_num = graph->getVerticesCount();

    std::queue<VertexID> bfs_queue;
    std::vector<bool> visited(vertex_num, false);

    tree = new TreeNode[vertex_num];
    for (unsigned i = 0; i < vertex_num; ++i)
    {
        tree[i].initialize(vertex_num);
    }
    bfs_order = new VertexID[vertex_num];

    unsigned visited_vertex_count = 0;
    bfs_queue.push(root_vertex);
    visited[root_vertex] = true;
    tree[root_vertex].level_ = 0;
    tree[root_vertex].id_ = root_vertex;

    while (!bfs_queue.empty())
    {
        const VertexID u = bfs_queue.front();
        bfs_queue.pop();
        bfs_order[visited_vertex_count++] = u;

        unsigned u_nbrs_count;
        const VertexID *u_nbrs = graph->getVertexInNeighbors(u, u_nbrs_count);
        for (unsigned i = 0; i < u_nbrs_count; ++i)
        {
            VertexID u_nbr = u_nbrs[i];

            if (!visited[u_nbr])
            {
                bfs_queue.push(u_nbr);
                visited[u_nbr] = true;
                tree[u_nbr].id_ = u_nbr;
                tree[u_nbr].parent_ = u;
                tree[u_nbr].level_ = tree[u].level_ + 1;
                tree[u].in_children_[tree[u].in_children_count_++] = u_nbr;
            }
        }
        u_nbrs = graph->getVertexOutNeighbors(u, u_nbrs_count);
        for (unsigned i = 0; i < u_nbrs_count; ++i)
        {
            VertexID u_nbr = u_nbrs[i];

            if (!visited[u_nbr])
            {
                bfs_queue.push(u_nbr);
                visited[u_nbr] = true;
                tree[u_nbr].id_ = u_nbr;
                tree[u_nbr].parent_ = u;
                tree[u_nbr].level_ = tree[u].level_ + 1;
                /* WARNNING: 这样求出的out_children_是不包含bi_children的，而in_children_是包含的，且没有写生成bi_children_是逻辑*/
                tree[u].out_children_[tree[u].out_children_count_++] = u_nbr;
            }
        }
    }
}

void GraphFeatures::weakBFS(const Graph *graph, VertexID root_vertex, TreeNode *&tree, VertexID *&bfs_order)
{
    unsigned vertex_num = graph->getVerticesCount();

    std::queue<VertexID> bfs_queue;
    std::vector<bool> visited(vertex_num, false);

    tree = new TreeNode[vertex_num];
    for (unsigned i = 0; i < vertex_num; ++i)
    {
        tree[i].initialize(vertex_num);
    }
    bfs_order = new VertexID[vertex_num];

    unsigned visited_vertex_count = 0;
    bfs_queue.push(root_vertex);
    visited[root_vertex] = true;
    tree[root_vertex].level_ = 0;
    tree[root_vertex].id_ = root_vertex;

    while (!bfs_queue.empty())
    {
        const VertexID u = bfs_queue.front();
        bfs_queue.pop();
        bfs_order[visited_vertex_count++] = u;

        unsigned u_nbrs_count;
        const VertexID *u_nbrs = graph->getVertexInNeighbors(u, u_nbrs_count);
        for (unsigned i = 0; i < u_nbrs_count; ++i)
        {
            VertexID u_nbr = u_nbrs[i];

            if (!visited[u_nbr])
            {
                bfs_queue.push(u_nbr);
                visited[u_nbr] = true;
                tree[u_nbr].id_ = u_nbr;
                tree[u_nbr].parent_ = u;
                tree[u_nbr].level_ = tree[u].level_ + 1;
                tree[u].in_children_[tree[u].in_children_count_++] = u_nbr;
            }
        }
        u_nbrs = graph->getVertexOutNeighbors(u, u_nbrs_count);
        for (unsigned i = 0; i < u_nbrs_count; ++i)
        {
            VertexID u_nbr = u_nbrs[i];

            if (!visited[u_nbr])
            {
                bfs_queue.push(u_nbr);
                visited[u_nbr] = true;
                tree[u_nbr].id_ = u_nbr;
                tree[u_nbr].parent_ = u;
                tree[u_nbr].level_ = tree[u].level_ + 1;
                /* WARNNING: 这样求出的out_children_是不包含bi_children的，而in_children_是包含的，且没有写生成bi_children_是逻辑*/
                tree[u].out_children_[tree[u].out_children_count_++] = u_nbr;
            }
        }
    }
#if STEP_DEBUG == 1
    std::cout << "vertex in generated bfs order:" << visited_vertex_count << std::endl;
#endif //STEP_DEBUG == 1
}

bool GraphFeatures::isWeakConnectedFromRoot(const Graph *graph, VertexID root_vertex)
{
    unsigned vertex_num = graph->getVerticesCount();

    std::queue<VertexID> bfs_queue;
    std::vector<bool> visited(vertex_num, false);

    unsigned visited_vertex_count = 0;
    bfs_queue.push(root_vertex);
    visited[root_vertex] = true;

    while (!bfs_queue.empty())
    {
        const VertexID u = bfs_queue.front();
        bfs_queue.pop();
        visited_vertex_count++;

        unsigned u_nbrs_count;
        const VertexID *u_nbrs = graph->getVertexInNeighbors(u, u_nbrs_count);
        for (unsigned i = 0; i < u_nbrs_count; ++i)
        {
            VertexID u_nbr = u_nbrs[i];

            if (!visited[u_nbr])
            {
                bfs_queue.push(u_nbr);
                visited[u_nbr] = true;
            }
        }
        u_nbrs = graph->getVertexOutNeighbors(u, u_nbrs_count);
        for (unsigned i = 0; i < u_nbrs_count; ++i)
        {
            VertexID u_nbr = u_nbrs[i];

            if (!visited[u_nbr])
            {
                bfs_queue.push(u_nbr);
                visited[u_nbr] = true;
            }
        }
    }
#if STEP_DEBUG == 1
    std::cout << "vertex in generated bfs order:" << visited_vertex_count << std::endl;
#endif //STEP_DEBUG == 1
    return visited_vertex_count == vertex_num;
}

void GraphFeatures::getKCore(const Graph *graph, int *core_table)
{
    int vertices_count = graph->getVerticesCount();
    int max_degree = graph->getGraphMaxDegree();

    int *vertices = new int[vertices_count];   // Vertices sorted by degree（按在联通子图中的度数；初始化为按graph中的度数降序）.
    int *position = new int[vertices_count];   // The position of vertices in vertices array.
    int *degree_bin = new int[max_degree + 1]; // Degree from 0 to max_degree. degree instance count
    int *offset = new int[max_degree + 1];     // The offset in vertices array according to degree.

    std::fill(degree_bin, degree_bin + (max_degree + 1), 0);

    for (int i = 0; i < vertices_count; ++i)
    {
        int degree = graph->getVertexDegree(i);
        core_table[i] = degree;
        degree_bin[degree] += 1;
    }

    int start = 0;
    for (int i = 0; i < max_degree + 1; ++i)
    {
        offset[i] = start;
        start += degree_bin[i];
    }
    //填vertices数组：按degree降序（桶排序的感觉）
    for (int i = 0; i < vertices_count; ++i)
    {
        int degree = graph->getVertexDegree(i);
        position[i] = offset[degree];
        vertices[position[i]] = i;
        offset[degree] += 1;
    }

    for (int i = max_degree; i > 0; --i)
    {
        offset[i] = offset[i - 1];
    }
    offset[0] = 0;
    //core_table[v]:v在联通子图中的度数
    for (int i = 0; i < vertices_count; ++i)
    {
        int v = vertices[i];

        unsigned count;

        /* WARNNING:in out邻居有交集，下述处理是否有问题？
        应该没有问题？（对于bi邻居，确实一个bi邻居就给中心顶点度数+2了） */
        const VertexID *neighbors = graph->getVertexInNeighbors(v, count);

        for (int j = 0; j < count; ++j)
        {
            int u = neighbors[j];
            //为啥如果下述为真则要core_table[u]--？把v从u所在的联通子图中删除？
            if (core_table[u] > core_table[v])
            {
                //即将core_table[u]--，所以先把u放到vertices的合适位置：
                //core_table[u]要-1，所以u要挪到vertices中其前一个度数组中：可以通过交换u和u当前所在度数组的第一个顶点来实现

                // Get the position and vertex which is with the same degree
                // and at the start position of vertices array.
                int cur_degree_u = core_table[u];
                int position_u = position[u];          //u在vertices数组中的下标（vertices数组：按degree降序）
                int position_w = offset[cur_degree_u]; //和u度数相同的点在vertices数组中的开始
                int w = vertices[position_w];

                if (u != w)
                {
                    // Swap u and w.
                    position[u] = position_w;
                    position[w] = position_u;
                    vertices[position_u] = w;
                    vertices[position_w] = u;
                }

                offset[cur_degree_u] += 1;
                core_table[u] -= 1;
            }
        }

        neighbors = graph->getVertexOutNeighbors(v, count);

        for (int j = 0; j < count; ++j)
        {
            int u = neighbors[j];
            //为啥如果下述为真则要core_table[u]--？把v从u所在的联通子图中删除？
            if (core_table[u] > core_table[v])
            {
                //即将core_table[u]--，所以先把u放到vertices的合适位置：
                //core_table[u]要-1，所以u要挪到vertices中其前一个度数组中：可以通过交换u和u当前所在度数组的第一个顶点来实现

                // Get the position and vertex which is with the same degree
                // and at the start position of vertices array.
                int cur_degree_u = core_table[u];
                int position_u = position[u];          //u在vertices数组中的下标（vertices数组：按degree降序）
                int position_w = offset[cur_degree_u]; //和u度数相同的点在vertices数组中的开始
                int w = vertices[position_w];

                if (u != w)
                {
                    // Swap u and w.
                    position[u] = position_w;
                    position[w] = position_u;
                    vertices[position_u] = w;
                    vertices[position_w] = u;
                }

                offset[cur_degree_u] += 1;
                core_table[u] -= 1;
            }
        }
    }

    delete[] vertices;
    delete[] position;
    delete[] degree_bin;
    delete[] offset;
}

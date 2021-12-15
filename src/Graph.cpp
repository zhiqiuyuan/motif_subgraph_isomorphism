#include "../inc/Graph.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string.h>

bool code_ascending(Lmtf l, Lmtf r)
{
    return l.code < r.code;
}
bool cnt_descending(Lmtf l, Lmtf r)
{
    return l.cnt > r.cnt;
}
void Graph::printGraphMetaData()
{
    std::cout << "|V|: " << vertices_count_ << ", |E|: " << edges_count_ << ", |label_set|: " << labels_count_ << std::endl;
    std::cout << "Max Degree: " << max_degree_ << ", Max Label Frequency: " << max_label_frequency_ << std::endl;
}

/*============================
LOAD GRAPH FROM FILE
*/
void Graph::BuildReverseIndex()
{
    reverse_index_ = new ui[vertices_count_];
    reverse_index_offsets_ = new ui[labels_count_ + 1];
    reverse_index_offsets_[0] = 0;

    ui total = 0;
    for (ui i = 0; i < labels_count_; ++i)
    {
        reverse_index_offsets_[i + 1] = total;
        total += labels_frequency_[i];
    }

    for (ui i = 0; i < vertices_count_; ++i)
    {
        LabelID label = labels_[i];
        reverse_index_[reverse_index_offsets_[label + 1]++] = i;
    }
}

#if DIRECTED_GRAPH == 1
#if OPTIMIZED_LABELED_GRAPH == 1
void Graph::BuildNLF_directed()
{
    in_nlf_ = new std::unordered_map<LabelID, ui>[vertices_count_];
    out_nlf_ = new std::unordered_map<LabelID, ui>[vertices_count_];
    bi_nlf_ = new std::unordered_map<LabelID, ui>[vertices_count_];
    const VertexID *neighbors;
    ui cnt;
    LabelID label;
    for (ui i = 0; i < vertices_count_; ++i)
    {
        //count 3 kind
        neighbors = getVertexInNeighbors(i, cnt);
        for (ui j = 0; j < cnt; ++j)
        {
            label = getVertexLabel(neighbors[j]);
            if (in_nlf_[i].count(label) == 0)
            {
                in_nlf_[i][label] = 0;
            }
            in_nlf_[i][label]++;
        }
        neighbors = getVertexOutNeighbors(i, cnt);
        for (ui j = 0; j < cnt; ++j)
        {
            label = getVertexLabel(neighbors[j]);
            if (out_nlf_[i].count(label) == 0)
            {
                out_nlf_[i][label] = 0;
            }
            out_nlf_[i][label]++;
        }
        neighbors = getVertexBiNeighbors(i, cnt);
        for (ui j = 0; j < cnt; ++j)
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
#endif //OPTIMIZED_LABELED_GRAPH == 1
//file_path:"xxx/name.graph"，将从xxx/name.graph中加载图结构
void Graph::loadGraphFromFile_directed(const std::string &file_path)
{
    std::ifstream infile(file_path);

    if (!infile.is_open())
    {
        std::cout << "Can not open the graph file " << file_path << " ." << std::endl;
        exit(-1);
    }

    char type;
    infile >> type >> vertices_count_ >> edges_count_;
    offsets_ = new ui[vertices_count_ + 1];
    offsets_[0] = 0;

    labels_ = new LabelID[vertices_count_];
    labels_count_ = 0;
    max_degree_ = 0; //max in+out degree

    in_neighbors_nums_ = new ui[vertices_count_]();
    in_neighbors_ = new LabelID[edges_count_ * 2];
    out_neighbors_nums_ = new ui[vertices_count_]();
    out_neighbors_ = new LabelID[edges_count_ * 2];
    bi_neighbors_nums_ = new ui[vertices_count_]();
    bi_neighbors_ = new LabelID[edges_count_ * 2];

    LabelID max_label_id = 0;

    while (infile >> type)
    {
        if (type == 'v')
        { // Read vertex.
            VertexID id;
            LabelID label;
            ui degree; //in+out degree
            infile >> id >> label >> degree;

            labels_[id] = label;
            offsets_[id + 1] = offsets_[id] + degree;

            if (degree > max_degree_)
            {
                max_degree_ = degree;
            }

            if (labels_frequency_.find(label) == labels_frequency_.end())
            {
                labels_frequency_[label] = 0;
                if (label > max_label_id)
                    max_label_id = label;
            }

            labels_frequency_[label] += 1;
        }
        else if (type == 'e')
        { // Read edge.
            VertexID begin;
            VertexID end;
            infile >> begin >> end;

            //begin
            ui offset = offsets_[begin] + out_neighbors_nums_[begin];
            out_neighbors_[offset] = end;
            out_neighbors_nums_[begin]++;

            //end
            offset = offsets_[end] + in_neighbors_nums_[end];
            in_neighbors_[offset] = begin;
            in_neighbors_nums_[end]++;
        }
    }

    infile.close();
    labels_count_ = (ui)labels_frequency_.size() > (max_label_id + 1) ? (ui)labels_frequency_.size() : max_label_id + 1;

    for (auto element : labels_frequency_)
    {
        if (element.second > max_label_frequency_)
        {
            max_label_frequency_ = element.second;
        }
    }

    //sort in out according vertex id (to support bi search)
    for (ui i = 0; i < vertices_count_; ++i)
    {
        std::sort(in_neighbors_ + offsets_[i], in_neighbors_ + offsets_[i] + in_neighbors_nums_[i]);
        std::sort(out_neighbors_ + offsets_[i], out_neighbors_ + offsets_[i] + out_neighbors_nums_[i]);
    }
    for (ui i = 0; i < vertices_count_; ++i)
    {
        ui off = offsets_[i];
        ui ii = off, oi = off;
        ui ie = ii + in_neighbors_nums_[i];
        ui oe = oi + out_neighbors_nums_[i];
        ui inid, outid;
        while (ii < ie && oi < oe)
        {
            inid = in_neighbors_[ii];
            outid = out_neighbors_[oi];
            if (inid < outid)
            {
                ++ii;
            }
            else if (inid > outid)
            {
                ++oi;
            }
            else
            {
                bi_neighbors_[off + bi_neighbors_nums_[i]] = inid;
                bi_neighbors_nums_[i]++;
                ++ii;
                ++oi;
            }
        }
    }

    BuildReverseIndex(); //label -> all vids
#if OPTIMIZED_LABELED_GRAPH == 1
    BuildNLF_directed();
#endif
}
#else
#if OPTIMIZED_LABELED_GRAPH == 1
void Graph::BuildNLF()
{
    nlf_ = new std::unordered_map<LabelID, ui>[vertices_count_];
    for (ui i = 0; i < vertices_count_; ++i)
    {
        ui count;
        const VertexID *neighbors = getVertexNeighbors(i, count);

        for (ui j = 0; j < count; ++j)
        {
            VertexID u = neighbors[j];
            LabelID label = getVertexLabel(u);
            if (nlf_[i].find(label) == nlf_[i].end())
            {
                nlf_[i][label] = 0;
            }

            nlf_[i][label] += 1;
        }
    }
}

#endif //OPTIMIZED_LABELED_GRAPH == 1
void Graph::loadGraphFromFile(const std::string &file_path)
{
    std::ifstream infile(file_path);

    if (!infile.is_open())
    {
        std::cout << "Can not open the graph file " << file_path << " ." << std::endl;
        exit(-1);
    }

    char type;
    infile >> type >> vertices_count_ >> edges_count_;
    offsets_ = new ui[vertices_count_ + 1];
    offsets_[0] = 0;

    neighbors_ = new VertexID[edges_count_ * 2];
    labels_ = new LabelID[vertices_count_];
    labels_count_ = 0;
    max_degree_ = 0;

    LabelID max_label_id = 0;
    std::vector<ui> neighbors_offset(vertices_count_, 0);

    while (infile >> type)
    {
        if (type == 'v')
        { // Read vertex.
            VertexID id;
            LabelID label;
            ui degree;
            infile >> id >> label >> degree;

            labels_[id] = label;
            offsets_[id + 1] = offsets_[id] + degree;

            if (degree > max_degree_)
            {
                max_degree_ = degree;
            }

            if (labels_frequency_.find(label) == labels_frequency_.end())
            {
                labels_frequency_[label] = 0;
                if (label > max_label_id)
                    max_label_id = label;
            }

            labels_frequency_[label] += 1;
        }
        else if (type == 'e')
        { // Read edge.
            VertexID begin;
            VertexID end;
            infile >> begin >> end;

            ui offset = offsets_[begin] + neighbors_offset[begin];
            neighbors_[offset] = end;

            offset = offsets_[end] + neighbors_offset[end];
            neighbors_[offset] = begin;

            neighbors_offset[begin] += 1;
            neighbors_offset[end] += 1;
        }
    }

    infile.close();
    labels_count_ = (ui)labels_frequency_.size() > (max_label_id + 1) ? (ui)labels_frequency_.size() : max_label_id + 1;

    for (auto element : labels_frequency_)
    {
        if (element.second > max_label_frequency_)
        {
            max_label_frequency_ = element.second;
        }
    }

    for (ui i = 0; i < vertices_count_; ++i)
    {
        std::sort(neighbors_ + offsets_[i], neighbors_ + offsets_[i + 1]);
    }
    BuildReverseIndex(); //label -> all vids
#if OPTIMIZED_LABELED_GRAPH == 1
    BuildNLF();
#endif
}
#endif //DIRECTED_GRAPH == 1 ELSE

/*============================
COUNT MOTIF DETAIL
*/
#if DIRECTED_GRAPH == 0 && TOPO_MOTIF_ENABLE == 1
void Graph::undirected_motif_count(ui i, ui &cnt)
{
    const ui *in;
    ui in_count,
        in = getVertexNeighbors(i, in_count);
    for (ui a = 0; a < in_count; ++a)
    {
        for (ui b = a + 1; b < in_count; ++b)
        {
            if (checkEdgeExistence(in[a], in[b]))
            {
                ++cnt;
            }
        }
    }
}
#endif //#if DIRECTED_GRAPH == 0 && TOPO_MOTIF_ENABLE==1

#if DIRECTED_GRAPH == 0 && LABEL_MOTIF_ENABLE == 1
void Graph::undirected_label_motif_count(ui i, std::map<ui, std::map<ui, ui>> *label_tri_map)
{
    const ui *neighbors;
    ui neighbors_num;
    neighbors = getVertexNeighbors(i, neighbors_num);
    //all neighors pairs: check edge exitense
    for (ui a = 0; a < neighbors_num; ++a)
    {
        for (ui b = a + 1; b < neighbors_num; ++b)
        {
            LabelID v1 = getVertexLabel(neighbors[a]), v2 = getVertexLabel(neighbors[b]);
            if (checkEdgeExistence(neighbors[a], neighbors[b]))
            {
                if (v1 > v2)
                {
                    ui tmp = v1;
                    v1 = v2;
                    v2 = tmp;
                }
                //v1->v2->++
                if ((*label_tri_map).count(v1) == 0 || (*label_tri_map)[v1].count(v2) == 0)
                {
                    (*label_tri_map)[v1][v2] == 0;
                }
                (*label_tri_map)[v1][v2]++;
            }
        }
    }
}
#endif //#if #if DIRECTED_GRAPH == 0 && LABEL_MOTIF_ENABLE==1

#if DIRECTED_GRAPH == 1 && TOPO_MOTIF_ENABLE == 1
void Graph::directed_motif_count(ui i, ui *motif_cnt)
{
    const ui *in, *out, *bi;
    ui in_count, out_count, bi_count;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    //out out
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = a + 1; b < out_count; ++b)
        {
            ui va = out[a];
            ui vb = out[b];
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ab || ba)
            {
                motif_cnt[1]++;
            }
            if (ab && ba)
            {
                motif_cnt[10]++;
            }
        }
    }
    //out in
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = 0; b < in_count; ++b)
        {
            ui va = out[a];
            ui vb = in[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    motif_cnt[5]++;
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    motif_cnt[2]++;
                }
                if (ab && ba)
                {
                    motif_cnt[8]++;
                }
            }
        }
    }
    //out bi
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = 0; b < bi_count; ++b)
        {
            ui va = out[a];
            ui vb = bi[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    motif_cnt[7]++;
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    motif_cnt[4]++;
                }
                if (ab && ba)
                {
                    motif_cnt[13]++;
                }
            }
        }
    }
    //in in
    for (ui a = 0; a < in_count; ++a)
    {
        for (ui b = a + 1; b < in_count; ++b)
        {
            ui va = in[a];
            ui vb = in[b];
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ba || ab)
            {
                motif_cnt[0]++;
            }
            if (ab && ba)
            {
                motif_cnt[3]++;
            }
        }
    }
    //in bi
    for (ui a = 0; a < in_count; ++a)
    {
        for (ui b = 0; b < bi_count; ++b)
        {
            ui va = in[a];
            ui vb = bi[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    motif_cnt[9]++;
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    motif_cnt[6]++;
                }
                if (ab && ba)
                {
                    motif_cnt[11]++;
                }
            }
        }
    }
    //bi bi
    for (ui a = 0; a < bi_count; ++a)
    {
        for (ui b = a + 1; b < bi_count; ++b)
        {
            ui va = bi[a];
            ui vb = bi[b];
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ab || ba)
            {
                motif_cnt[12]++;
            }
            if (ab && ba)
            {
                motif_cnt[14]++;
            }
        }
    }
}

void Graph::directed_motif_count_debug(ui i, ui *motif_cnt, std::vector<std::vector<std::pair<int, int>>> &motif_detail)
{
    const ui *in, *out, *bi;
    ui in_count, out_count, bi_count;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    //out out
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = a + 1; b < out_count; ++b)
        {
            ui va = out[a];
            ui vb = out[b];
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ab || ba)
            {
                motif_cnt[1]++;
                motif_detail[1].push_back(std::pair<int, int>(va, vb));
            }
            if (ab && ba)
            {
                motif_cnt[10]++;
                motif_detail[10].push_back(std::pair<int, int>(va, vb));
            }
        }
    }
    //out in
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = 0; b < in_count; ++b)
        {
            ui va = out[a];
            ui vb = in[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    motif_cnt[5]++;
                    motif_detail[5].push_back(std::pair<int, int>(va, vb));
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    motif_cnt[2]++;
                    motif_detail[2].push_back(std::pair<int, int>(va, vb));
                }
                if (ab && ba)
                {
                    motif_cnt[8]++;
                    motif_detail[8].push_back(std::pair<int, int>(va, vb));
                }
            }
        }
    }
    //out bi
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = 0; b < bi_count; ++b)
        {
            ui va = out[a];
            ui vb = bi[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    motif_cnt[7]++;
                    motif_detail[7].push_back(std::pair<int, int>(va, vb));
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    motif_cnt[4]++;
                    motif_detail[4].push_back(std::pair<int, int>(va, vb));
                }
                if (ab && ba)
                {
                    motif_cnt[13]++;
                    motif_detail[13].push_back(std::pair<int, int>(va, vb));
                }
            }
        }
    }
    //in in
    for (ui a = 0; a < in_count; ++a)
    {
        for (ui b = a + 1; b < in_count; ++b)
        {
            ui va = in[a];
            ui vb = in[b];
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ba || ab)
            {
                motif_cnt[0]++;
                motif_detail[0].push_back(std::pair<int, int>(va, vb));
            }
            if (ab && ba)
            {
                motif_cnt[3]++;
                motif_detail[3].push_back(std::pair<int, int>(va, vb));
            }
        }
    }
    //in bi
    for (ui a = 0; a < in_count; ++a)
    {
        for (ui b = 0; b < bi_count; ++b)
        {
            ui va = in[a];
            ui vb = bi[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    motif_cnt[9]++;
                    motif_detail[9].push_back(std::pair<int, int>(va, vb));
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    motif_cnt[6]++;
                    motif_detail[6].push_back(std::pair<int, int>(va, vb));
                }
                if (ab && ba)
                {
                    motif_cnt[11]++;
                    motif_detail[11].push_back(std::pair<int, int>(va, vb));
                }
            }
        }
    }
    //bi bi
    for (ui a = 0; a < bi_count; ++a)
    {
        for (ui b = a + 1; b < bi_count; ++b)
        {
            ui va = bi[a];
            ui vb = bi[b];
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ab || ba)
            {
                motif_cnt[12]++;
                motif_detail[12].push_back(std::pair<int, int>(va, vb));
            }
            if (ab && ba)
            {
                motif_cnt[14]++;
                motif_detail[14].push_back(std::pair<int, int>(va, vb));
            }
        }
    }
}
#endif //#if DIRECTED_GRAPH == 0 && LABEL_MOTIF_ENABLE==1

#if DIRECTED_GRAPH == 1 && LABEL_MOTIF_ENABLE == 1
void Graph::directed_label_motif_count(ui i, std::map<ui, ui> *cnt_map_arr)
{
    const ui *in, *out, *bi, *_in, *_out, *_bi;
    ui in_count, out_count, bi_count, _in_count, _out_count, _bi_count;
    ui pre_code, pre_code_pre, data_vertex;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    for (ui j = 0; j < MOTIF_COUNT_DEMENSION; ++j)
    {
        cnt_map_arr[j].clear();
    }
    //in
    for (ui _ = 0; _ < in_count; ++_)
    {
        data_vertex = in[_];
        pre_code_pre = (getVertexLabel(data_vertex) << LABEL_BIT_WIDTH);
        _in = getVertexInNeighbors(data_vertex, _in_count);
        _out = getVertexOutNeighbors(data_vertex, _out_count);
        _bi = getVertexBiNeighbors(data_vertex, _bi_count);
        //in
        for (ui a = 0; a < _in_count; ++a)
        {
            data_vertex = _in[a];
            if (data_vertex == i)
            {
                continue;
            }
            pre_code = pre_code_pre | getVertexLabel(data_vertex);
            if (cnt_map_arr[0].count(pre_code) == 0)
            {
                cnt_map_arr[0][pre_code] = 1;
            }
            else
            {
                ++cnt_map_arr[0][pre_code];
            }
            if (checkEdgeExistence_strict(data_vertex, i))
            {
                if (cnt_map_arr[6].count(pre_code) == 0)
                {
                    cnt_map_arr[6][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[6][pre_code];
                }
            }
        }
        //out
        for (ui a = 0; a < _out_count; ++a)
        {
            data_vertex = _out[a];
            if (data_vertex == i)
            {
                continue;
            }
            pre_code = pre_code_pre | getVertexLabel(data_vertex);
            if (cnt_map_arr[2].count(pre_code) == 0)
            {
                cnt_map_arr[2][pre_code] = 1;
            }
            else
            {
                ++cnt_map_arr[2][pre_code];
            }
            if (checkEdgeExistence_strict(i, data_vertex))
            {
                if (cnt_map_arr[7].count(pre_code) == 0)
                {
                    cnt_map_arr[7][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[7][pre_code];
                }
            }
        }
        //bi
        for (ui a = 0; a < _bi_count; ++a)
        {
            data_vertex = _bi[a];
            if (data_vertex == i)
            {
                continue;
            }
            pre_code = pre_code_pre | getVertexLabel(data_vertex);
            if (cnt_map_arr[11].count(pre_code) == 0)
            {
                cnt_map_arr[11][pre_code] = 1;
            }
            else
            {
                ++cnt_map_arr[11][pre_code];
            }
            if (checkEdgeExistence_strict(data_vertex, i))
            {
                if (cnt_map_arr[13].count(pre_code) == 0)
                {
                    cnt_map_arr[13][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[13][pre_code];
                }
            }
            if (checkEdgeExistence_bi(i, data_vertex))
            {
                if (cnt_map_arr[20].count(pre_code) == 0)
                {
                    cnt_map_arr[20][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[20][pre_code];
                }
            }
        }
    }
    //out
    for (ui _ = 0; _ < out_count; ++_)
    {
        data_vertex = out[_];
        pre_code_pre = (getVertexLabel(data_vertex) << LABEL_BIT_WIDTH);
        _in = getVertexInNeighbors(data_vertex, _in_count);
        _out = getVertexOutNeighbors(data_vertex, _out_count);
        _bi = getVertexBiNeighbors(data_vertex, _bi_count);
        //in
        for (ui a = 0; a < _in_count; ++a)
        {
            data_vertex = _in[a];
            if (data_vertex == i)
            {
                continue;
            }
            pre_code = pre_code_pre | getVertexLabel(data_vertex);
            if (cnt_map_arr[3].count(pre_code) == 0)
            {
                cnt_map_arr[3][pre_code] = 1;
            }
            else
            {
                ++cnt_map_arr[3][pre_code];
            }
            if (checkEdgeExistence_strict(i, data_vertex))
            {
                if (cnt_map_arr[5].count(pre_code) == 0)
                {
                    cnt_map_arr[5][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[5][pre_code];
                }
            }
        }
        //out
        for (ui a = 0; a < _out_count; ++a)
        {
            data_vertex = _out[a];
            if (data_vertex == i)
            {
                continue;
            }
            pre_code = pre_code_pre | getVertexLabel(data_vertex);
            if (cnt_map_arr[1].count(pre_code) == 0)
            {
                cnt_map_arr[1][pre_code] = 1;
            }
            else
            {
                ++cnt_map_arr[1][pre_code];
            }
            if (checkEdgeExistence_strict(data_vertex, i))
            {
                if (cnt_map_arr[4].count(pre_code) == 0)
                {
                    cnt_map_arr[4][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[4][pre_code];
                }
            }
            if (checkEdgeExistence_bi(i, data_vertex))
            {
                if (cnt_map_arr[19].count(pre_code) == 0)
                {
                    cnt_map_arr[19][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[19][pre_code];
                }
            }
        }
        //bi
        for (ui a = 0; a < _bi_count; ++a)
        {
            data_vertex = _bi[a];
            if (data_vertex == i)
            {
                continue;
            }
            pre_code = pre_code_pre | getVertexLabel(data_vertex);
            if (cnt_map_arr[9].count(pre_code) == 0)
            {
                cnt_map_arr[9][pre_code] = 1;
            }
            else
            {
                ++cnt_map_arr[9][pre_code];
            }
            if (checkEdgeExistence_strict(i, data_vertex))
            {
                if (cnt_map_arr[15].count(pre_code) == 0)
                {
                    cnt_map_arr[15][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[15][pre_code];
                }
            }
            if (checkEdgeExistence_strict(data_vertex, i))
            {
                if (cnt_map_arr[18].count(pre_code) == 0)
                {
                    cnt_map_arr[18][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[18][pre_code];
                }
            }
        }
    }
    //bi
    for (ui _ = 0; _ < bi_count; ++_)
    {
        data_vertex = bi[_];
        pre_code_pre = (getVertexLabel(data_vertex) << LABEL_BIT_WIDTH);
        _in = getVertexInNeighbors(data_vertex, _in_count);
        _out = getVertexOutNeighbors(data_vertex, _out_count);
        _bi = getVertexBiNeighbors(data_vertex, _bi_count);
        //in
        for (ui a = 0; a < _in_count; ++a)
        {
            data_vertex = _in[a];
            if (data_vertex == i)
            {
                continue;
            }
            pre_code = pre_code_pre | getVertexLabel(data_vertex);
            if (cnt_map_arr[8].count(pre_code) == 0)
            {
                cnt_map_arr[8][pre_code] = 1;
            }
            else
            {
                ++cnt_map_arr[8][pre_code];
            }
            if (checkEdgeExistence_strict(data_vertex, i))
            {
                if (cnt_map_arr[16].count(pre_code) == 0)
                {
                    cnt_map_arr[16][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[16][pre_code];
                }
            }
            if (checkEdgeExistence_bi(i, data_vertex))
            {
                if (cnt_map_arr[21].count(pre_code) == 0)
                {
                    cnt_map_arr[21][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[21][pre_code];
                }
            }
        }
        //out
        for (ui a = 0; a < _out_count; ++a)
        {
            data_vertex = _out[a];
            if (data_vertex == i)
            {
                continue;
            }
            pre_code = pre_code_pre | getVertexLabel(data_vertex);
            if (cnt_map_arr[10].count(pre_code) == 0)
            {
                cnt_map_arr[10][pre_code] = 1;
            }
            else
            {
                ++cnt_map_arr[10][pre_code];
            }
            if (checkEdgeExistence_strict(i, data_vertex))
            {
                if (cnt_map_arr[14].count(pre_code) == 0)
                {
                    cnt_map_arr[14][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[14][pre_code];
                }
            }
            if (checkEdgeExistence_strict(data_vertex, i))
            {
                if (cnt_map_arr[17].count(pre_code) == 0)
                {
                    cnt_map_arr[17][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[17][pre_code];
                }
            }
        }
        //bi
        for (ui a = 0; a < _bi_count; ++a)
        {
            data_vertex = _bi[a];
            if (data_vertex == i)
            {
                continue;
            }
            pre_code = pre_code_pre | getVertexLabel(data_vertex);
            if (cnt_map_arr[12].count(pre_code) == 0)
            {
                cnt_map_arr[12][pre_code] = 1;
            }
            else
            {
                ++cnt_map_arr[12][pre_code];
            }
            if (checkEdgeExistence_strict(i, data_vertex))
            {
                if (cnt_map_arr[22].count(pre_code) == 0)
                {
                    cnt_map_arr[22][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[22][pre_code];
                }
            }
            if (checkEdgeExistence_bi(i, data_vertex))
            {
                if (cnt_map_arr[23].count(pre_code) == 0)
                {
                    cnt_map_arr[23][pre_code] = 1;
                }
                else
                {
                    ++cnt_map_arr[23][pre_code];
                }
            }
        }
    }
}
//choose k lmotif; lmtf_arr sorted
//impossible_code_entry_num:如果有mid方法，则通过此返回k-std::min(k, lmtf_arr_sz)，即对齐部分有多少entry，用于给mid先在有效部分选
void Graph::choose_k_from_lmtf_arr(Lmtf *lmtf_arr, ui lmtf_arr_sz, ui k, Lmtf **lmtf_arr_G, std::string *kmethod_set, ui kmethod_num, int &impossible_code_entry_num)
{
    ui impossible_code = (1 << (2 * LABEL_BIT_WIDTH));
    ui j;
    std::string name;
    for (ui i = 0; i < kmethod_num; ++i)
    {
        name = kmethod_set[i];
        if (name == "top")
        {
            for (j = 0; j < std::min(k, lmtf_arr_sz); ++j)
            {
                lmtf_arr_G[i][j] = lmtf_arr[j];
            }
            for (; j < k; ++j)
            {
                lmtf_arr_G[i][j].code = impossible_code;
                lmtf_arr_G[i][j].cnt = 0;
            }
        }
        else if (name == "down")
        {
            for (j = 0; j < std::min(k, lmtf_arr_sz); ++j)
            {
                lmtf_arr_G[i][j] = lmtf_arr[lmtf_arr_sz - j - 1];
            }
            for (; j < k; ++j)
            {
                lmtf_arr_G[i][j].code = impossible_code;
                lmtf_arr_G[i][j].cnt = 0;
            }
        }
        else if (name == "mid")
        {
            int e = k / 2; //k[-k/2,k/2] 2[-1,1] 3[-1,1]
            if (k % 2 == 0)
            {
                //K偶数：无法对称（0），约定左边多取一个(since cnt descending)
                --e;
            }
            int mid = (((int)lmtf_arr_sz)) / 2; //lmtf_arr_sz奇数：中间 偶数：中间偏右
            e = std::min(mid + e, ((int)lmtf_arr_sz) - 1);
            int jj;
            int b = std::max(mid - ((int)k / 2), 0);
            //std::cout << "vertexi: " << i << " k:" << k << " lmtf_arr_sz: " << lmtf_arr_sz << " b:" << b << " e:" << e << std::endl;
            for (jj = b; jj <= e; ++jj)
            {
                lmtf_arr_G[i][jj - b] = lmtf_arr[jj];
            }
            /*
            for (jj -= b; jj < k; ++jj)
            {
                lmtf_arr_G[i][jj].code = impossible_code;
                lmtf_arr_G[i][jj].cnt = 0;
            }
            */

            jj -= b;
            impossible_code_entry_num = (int)k - jj;
        }
        else if (name == "rand")
        {
            if (k >= lmtf_arr_sz)
            {
                for (j = 0; j < lmtf_arr_sz; ++j)
                {
                    lmtf_arr_G[i][j] = lmtf_arr[j];
                }
                for (; j < k; ++j)
                {
                    lmtf_arr_G[i][j].code = impossible_code;
                    lmtf_arr_G[i][j].cnt = 0;
                }
            }
            else
            {
                std::set<ui> idx_set;
                ui idx;
                while (idx_set.size() < k)
                {
                    idx = rand() % lmtf_arr_sz;
                    idx_set.insert(idx);
                }
                j = 0;
                for (ui id : idx_set)
                {
                    lmtf_arr_G[i][j] = lmtf_arr[id];
                    ++j;
                }
            }
        }
        else
        {
            std::cout << "KMETHOD NAME ERROR: only {top, down, mid, rand} are allowed" << std::endl;
            return;
        }
    }

    //sort:code_ascending
    for (j = 0; j < kmethod_num; ++j)
    {
        std::sort(lmtf_arr_G[j], lmtf_arr_G[j] + std::min(k, lmtf_arr_sz), code_ascending);
    }
}
#endif //#if DIRECTED_GRAPH == 1 && LABEL_MOTIF_ENABLE==1

#if LABEL_MOTIF_LIMIT == 1
//for vertex i
//value in one_vertex_cnt_map needs to be already specified
//memory it points to needs to be already allocated
void Graph::directed_label_motif_count_limit(ui i, std::map<ui, ui> *one_vertex_cnt_map)
{
    const ui *in, *out, *bi;
    ui in_count, out_count, bi_count;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    ui code, label_code_seg;

    //out out
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = a + 1; b < out_count; ++b)
        {
            ui va = out[a];
            ui vb = out[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ab)
            {
                code = 1u << LABEL_SEG_WIDTH;
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
            }
            if (ba)
            {
                code = 1u << LABEL_SEG_WIDTH;
                code |= (alb << LABEL_BIT_WIDTH);
                code |= blb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
            }
            if (ab && ba)
            {
                code = 10u << LABEL_SEG_WIDTH;
                if (alb < blb)
                {
                    ui tmp = alb;
                    alb = blb;
                    blb = tmp;
                }
                //blb<=alb
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
            }
        }
    }
    //out in
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = 0; b < in_count; ++b)
        {
            ui va = out[a];
            ui vb = in[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    code = 5u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    code = 2u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                }
                if (ab && ba)
                {
                    code = 8u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                }
            }
        }
    }
    //out bi
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = 0; b < bi_count; ++b)
        {
            ui va = out[a];
            ui vb = bi[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    code = 7u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    code = 4u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                }
                if (ab && ba)
                {
                    code = 13u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                }
            }
        }
    }
    //in in
    for (ui a = 0; a < in_count; ++a)
    {
        for (ui b = a + 1; b < in_count; ++b)
        {
            ui va = in[a];
            ui vb = in[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ba)
            {
                code = 0u << LABEL_SEG_WIDTH;
                code |= (alb << LABEL_BIT_WIDTH);
                code |= blb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
            }
            if (ab)
            {
                code = 0u << LABEL_SEG_WIDTH;
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
            }
            if (ab && ba)
            {
                code = 3u << LABEL_SEG_WIDTH;
                if (alb < blb)
                {
                    ui tmp = alb;
                    alb = blb;
                    blb = tmp;
                }
                //blb<=alb
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
            }
        }
    }
    //in bi
    for (ui a = 0; a < in_count; ++a)
    {
        for (ui b = 0; b < bi_count; ++b)
        {
            ui va = in[a];
            ui vb = bi[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    code = 9u << LABEL_SEG_WIDTH;
                    code |= (alb << LABEL_BIT_WIDTH);
                    code |= blb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    code = 6u << LABEL_SEG_WIDTH;
                    code |= (alb << LABEL_BIT_WIDTH);
                    code |= blb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                }
                if (ab && ba)
                {
                    code = 11u << LABEL_SEG_WIDTH;
                    code |= (alb << LABEL_BIT_WIDTH);
                    code |= blb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                }
            }
        }
    }
    //bi bi
    for (ui a = 0; a < bi_count; ++a)
    {
        for (ui b = a + 1; b < bi_count; ++b)
        {
            ui va = bi[a];
            ui vb = bi[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ab)
            {
                code = 12u << LABEL_SEG_WIDTH;
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
            }
            if (ba)
            {
                code = 12u << LABEL_SEG_WIDTH;
                code |= (alb << LABEL_BIT_WIDTH);
                code |= blb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
            }
            if (ab && ba)
            {
                code = 14u << LABEL_SEG_WIDTH;
                if (alb < blb)
                {
                    ui tmp = alb;
                    alb = blb;
                    blb = tmp;
                }
                //blb<=alb
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
            }
        }
    }
}

void Graph::directed_label_motif_count_limit_debug(ui i, std::map<ui, ui> *one_vertex_cnt_map, std::map<ui, std::vector<ui>> *code2vertexvertex)
{
    const ui *in, *out, *bi;
    ui in_count, out_count, bi_count;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    ui code, label_code_seg;
    //out out
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = a + 1; b < out_count; ++b)
        {
            ui va = out[a];
            ui vb = out[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ab)
            {
                code = 1u << LABEL_SEG_WIDTH;
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
                (*code2vertexvertex)[code].push_back((vb << 16) | va);
            }
            if (ba)
            {
                code = 1u << LABEL_SEG_WIDTH;
                code |= (alb << LABEL_BIT_WIDTH);
                code |= blb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
                (*code2vertexvertex)[code].push_back((va << 16) | vb);
            }
            if (ab && ba)
            {
                code = 10u << LABEL_SEG_WIDTH;
                if (alb < blb)
                {
                    ui tmp = alb;
                    alb = blb;
                    blb = tmp;
                }
                //blb<=alb
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
                (*code2vertexvertex)[code].push_back((vb << 16) | va);
            }
        }
    }
    //out in
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = 0; b < in_count; ++b)
        {
            ui va = out[a];
            ui vb = in[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    code = 5u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                    (*code2vertexvertex)[code].push_back((vb << 16) | va);
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    code = 2u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                    (*code2vertexvertex)[code].push_back((vb << 16) | va);
                }
                if (ab && ba)
                {
                    code = 8u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                    (*code2vertexvertex)[code].push_back((vb << 16) | va);
                }
            }
        }
    }
    //out bi
    for (ui a = 0; a < out_count; ++a)
    {
        for (ui b = 0; b < bi_count; ++b)
        {
            ui va = out[a];
            ui vb = bi[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    code = 7u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                    (*code2vertexvertex)[code].push_back((vb << 16) | va);
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    code = 4u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                    (*code2vertexvertex)[code].push_back((va << 16) | vb);
                }
                if (ab && ba)
                {
                    code = 13u << LABEL_SEG_WIDTH;
                    code |= (blb << LABEL_BIT_WIDTH);
                    code |= alb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                    (*code2vertexvertex)[code].push_back((vb << 16) | va);
                }
            }
        }
    }
    //in in
    for (ui a = 0; a < in_count; ++a)
    {
        for (ui b = a + 1; b < in_count; ++b)
        {
            ui va = in[a];
            ui vb = in[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ba)
            {
                code = 0u << LABEL_SEG_WIDTH;
                code |= (alb << LABEL_BIT_WIDTH);
                code |= blb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
                (*code2vertexvertex)[code].push_back((va << 16) | vb);
            }
            if (ab)
            {
                code = 0u << LABEL_SEG_WIDTH;
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
                (*code2vertexvertex)[code].push_back((vb << 16) | va);
            }
            if (ab && ba)
            {
                code = 3u << LABEL_SEG_WIDTH;
                if (alb < blb)
                {
                    ui tmp = alb;
                    alb = blb;
                    blb = tmp;
                }
                //blb<=alb
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
                (*code2vertexvertex)[code].push_back((vb << 16) | va);
            }
        }
    }
    //in bi
    for (ui a = 0; a < in_count; ++a)
    {
        for (ui b = 0; b < bi_count; ++b)
        {
            ui va = in[a];
            ui vb = bi[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence_strict(va, vb);
                if (ab)
                {
                    code = 9u << LABEL_SEG_WIDTH;
                    code |= (alb << LABEL_BIT_WIDTH);
                    code |= blb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                    (*code2vertexvertex)[code].push_back((va << 16) | vb);
                }
                bool ba = checkEdgeExistence_strict(vb, va);
                if (ba)
                {
                    code = 6u << LABEL_SEG_WIDTH;
                    code |= (alb << LABEL_BIT_WIDTH);
                    code |= blb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                    (*code2vertexvertex)[code].push_back((va << 16) | vb);
                }
                if (ab && ba)
                {
                    code = 11u << LABEL_SEG_WIDTH;
                    code |= (alb << LABEL_BIT_WIDTH);
                    code |= blb;
                    if (one_vertex_cnt_map->count(code) == 0)
                    {
                        (*one_vertex_cnt_map)[code] = 0;
                    }
                    ++(*one_vertex_cnt_map)[code];
                    (*code2vertexvertex)[code].push_back((va << 16) | vb);
                }
            }
        }
    }
    //bi bi
    for (ui a = 0; a < bi_count; ++a)
    {
        for (ui b = a + 1; b < bi_count; ++b)
        {
            ui va = bi[a];
            ui vb = bi[b];
            ui alb = getVertexLabel(va);
            ui blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence_strict(va, vb);
            bool ba = checkEdgeExistence_strict(vb, va);
            if (ab)
            {
                code = 12u << LABEL_SEG_WIDTH;
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
                (*code2vertexvertex)[code].push_back((vb << 16) | va);
            }
            if (ba)
            {
                code = 12u << LABEL_SEG_WIDTH;
                code |= (alb << LABEL_BIT_WIDTH);
                code |= blb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
                (*code2vertexvertex)[code].push_back((va << 16) | vb);
            }
            if (ab && ba)
            {
                code = 14u << LABEL_SEG_WIDTH;
                if (alb < blb)
                {
                    ui tmp = alb;
                    alb = blb;
                    blb = tmp;
                }
                //blb<=alb
                code |= (blb << LABEL_BIT_WIDTH);
                code |= alb;
                if (one_vertex_cnt_map->count(code) == 0)
                {
                    (*one_vertex_cnt_map)[code] = 0;
                }
                ++(*one_vertex_cnt_map)[code];
                (*code2vertexvertex)[code].push_back((vb << 16) | va);
            }
        }
    }
}

#endif //LABEL_MOTIF_LIMIT==1

/*============================
COUNT MOTIF INTERFACE
*/
#if TOPO_MOTIF_ENABLE == 1
//filename_prefix:xxx/youtube（没有.graph）
//离线：结果写入xxx/youtube_directed.txt或者xxx/youtube_undirected.txt
//在线离线：如果ONLINE_WRITE_FILE_DEBUG则结果会写入xxx/youtube_directed_debug.txt
void Graph::generateMotifCount(std::string filename_prefix)
{
#if DIRECTED_GRAPH == 0
    filename_prefix += "_undirected";
#else
    filename_prefix += "_directed"; //xxx/youtube_directed
#endif // DIRECTED_GRAPH
#if ONLINE_STAGE == 0
    std::ofstream fout(filename_prefix + ".txt");
    if (!fout.is_open())
    {
        std::cout << "file not exited: " << filename_prefix + ".txt" << std::endl;
        return;
    }
#endif //ONLINE_STAGE == 0
#if WRITE_TO_FILE_DEBUG == 1
    std::ofstream fout_debug(filename_prefix + "_debug.txt");
    if (!fout_debug.is_open())
    {
        std::cout << "file not exited: " << filename_prefix + ".txt" << std::endl;
        return;
    }
#endif //#if WRITE_TO_FILE_DEBUG == 1

#if DIRECTED_GRAPH == 1
    ui *motif_cnt;
#if ONLINE_STAGE == 1
    motif_count_ = new ui *[vertices_count_];
    for (size_t i = 0; i < vertices_count_; ++i)
    {
        motif_count_[i] = new ui[MOTIF_COUNT_DEMENSION]();
    }
#else  //offline: no need for keep
    motif_cnt = new ui[MOTIF_COUNT_DEMENSION]();
#endif // ONLINE_STAGE

    for (ui i = 0; i < vertices_count_; ++i)
    { //i:vid
#if ONLINE_STAGE == 1
        motif_cnt = motif_count_[i];
#else
        memset(motif_cnt, 0, sizeof(unsigned) * MOTIF_COUNT_DEMENSION);
#endif //ONLINE_STAGE
#if WRITE_TO_FILE_DEBUG == 1
        std::vector<std::vector<std::pair<int, int>>> motif_detail(MOTIF_COUNT_DEMENSION, std::vector<std::pair<int, int>>());
        directed_motif_count_debug(i, motif_cnt, motif_detail);
#else  //WRITE_TO_FILE_DEBUG == 0
        directed_motif_count(i, motif_cnt);
#endif //#if WRITE_TO_FILE_DEBUG == 1
        char split_c;
#if ONLINE_STAGE == 0
        split_c = ' ';
        fout << i;
        for (ui a = 0; a < MOTIF_COUNT_DEMENSION; ++a)
        {
            fout << split_c << motif_cnt[a];
        }
        fout << "\n";
#endif //#if ONLINE_STAGE == 0
#if WRITE_TO_FILE_DEBUG == 1
        split_c = '\t';
        fout_debug << "v" << i + 1 << "\n";
        for (ui a = 0; a < MOTIF_COUNT_DEMENSION; ++a)
        {
            fout_debug << a << ": cnt" << motif_cnt[a] << ": ";
            for (auto p : motif_detail[a])
            {
                //elem:pair
                fout_debug << "(v" << p.first + 1 << " ,v" << p.second + 1 << ")" << split_c;
            }
            fout_debug << "\n";
        }
        fout_debug << "\n";
#endif //#if WRITE_TO_FILE_DEBUG == 1
        if (i % 100 == 0)
        {
            std::cout << "already finished motif count for " << i << " nodes" << std::endl;
        }
    }
#if ONLINE_STAGE == 0
    delete[] motif_cnt;
#endif
#else
#if ONLINE_STAGE == 1
    tri_count_ = new ui[vertices_count_]();
#endif //#if ONLINE_STAGE == 1
    for (ui i = 0; i < vertices_count_; ++i)
    {
        ui cnt = 0;
        undirected_motif_count(i, cnt);
#if ONLINE_STAGE == 1
        tri_count_[i] = cnt;
#endif //#if ONLINE_STAGE == 1
        fout << i << " " << cnt << "\n";
        //std::cout<<"already finished motif count for "<<i<<" nodes"<<std::endl;
    }
#endif // DIRECTED_GRAPH

#if ONLINE_STAGE == 0
    fout.close();
#endif //#if ONLINE_STAGE == 0
#if WRITE_TO_FILE_DEBUG == 1
    fout_debug.close();
#endif //#if WRITE_TO_FILE_DEBUG == 1
}
#endif //TOPO_MOTIF_ENABLE==1

#if LABEL_MOTIF_ENABLE == 1
//label_directed_online 有向图DEBUG才写文件
//label_undirected_online,label_undirected_offline 无向图都写文件，DEBUG才写CodeInChar文件
//filename_prefix:"xxx/youtube"，将把motif写入到xxx/youtube_directed_label.txt中
void Graph::generateMotifCount_label(std::string filename_prefix)
{
    /*
    OPEN FILE
    */
#if DIRECTED_GRAPH == 0 //label_undirected_online,label_undirected_offline
    filename_prefix += "_undirected_label";
    std::ofstream fout(filename_prefix + ".txt");
    if (!fout.is_open())
    {
        std::cout << "file not exited: " << filename_prefix + ".txt" << std::endl;
        return;
    }
#else //label_directed_online
    filename_prefix += "_directed_label";
#if WRITE_TO_FILE_DEBUG == 1
    std::ofstream fout(filename_prefix + ".txt");
    if (!fout.is_open())
    {
        std::cout << "file not exited: " << filename_prefix + ".txt" << std::endl;
        return;
    }
#endif //WRITE_TO_FILE_DEBUG==1
#endif // DIRECTED_GRAPH

#if WRITE_TO_FILE_DEBUG == 1
    std::ofstream fout_codeInChar(filename_prefix + "_codeInChar.txt");
    if (!fout_codeInChar.is_open())
    {
        std::cout << "file not exited: " << filename_prefix + "_codeInChar.txt" << std::endl;
        return;
    }
#endif //WRITE_TO_FILE_DEBUG==1

    /*
    COUNT MOTIF
    */
#if DIRECTED_GRAPH == 0 //label_undirected_online,label_undirected_offline
    std::map<ui, std::map<ui, ui>> *label_tri_map;
#if ONLINE_STAGE == 1
    label_tri_count_ = new std::map<ui, std::map<ui, ui>>[vertices_count_];
#else
    label_tri_map = new std::map<ui, std::map<ui, ui>>;
#endif //#if ONLINE_STAGE == 1
    //gen AB tri count in ego network for each vertex
    for (ui i = 0; i < vertices_count_; ++i)
    {
#if ONLINE_STAGE == 0
        (*label_tri_map).clear();
#else
        label_tri_map = label_tri_count_ + i;
#endif //#if ONLINE_STAGE == 1
        undirected_label_motif_count(i, label_tri_map);
        fout << i << "\n";
        for (auto item : *label_tri_map)
        {
            for (auto it : item.second)
            {
                fout << item.first << " " << it.first << " " << it.second << "\n";
            }
        }
        //std::cout << "already finished motif label tri count for " << i << " nodes" << std::endl;
    }
#if ONLINE_STAGE == 0
    delete label_tri_map;
#endif //#if ONLINE_STAGE == 1

#else //label_directed_online
    std::map<ui, ui> *cnt_map_arr = new std::map<ui, ui>[MOTIF_COUNT_DEMENSION];
    Lmtf *lmtf_arr;
    label_motif_count_ = new Lmtf *[vertices_count_];
    label_motif_count_sz_ = new ui[vertices_count_];
    ui lmtf_arr_sz;
    ui pre_code, pre_code_pre, data_vertex;
    ui max_lmtf_kind_cnt = 0, min_lmtf_kind_cnt = -1;

    char spilt_c = ' ';
#if WRITE_TO_FILE_DEBUG == 1
    spilt_c = '\t';
#endif                        //WRITE_TO_FILE_DEBUG==1
    for (ui i = 0; i < vertices_count_; ++i)
    {
        //i:vid
        directed_label_motif_count(i, cnt_map_arr);
        /*
        online: turn struct arr and sort(ascending code) respectively
        */
        lmtf_arr_sz = 0;
        for (ui j = 0; j < MOTIF_COUNT_DEMENSION; ++j)
        {
            lmtf_arr_sz += cnt_map_arr[j].size();
        }
        if (lmtf_arr_sz > max_lmtf_kind_cnt)
        {
            max_lmtf_kind_cnt = lmtf_arr_sz;
        }
        if (lmtf_arr_sz < min_lmtf_kind_cnt)
        {
            min_lmtf_kind_cnt = lmtf_arr_sz;
        }

        label_motif_count_[i] = new Lmtf[lmtf_arr_sz];
        //label_motif_count_sz_[i] = lmtf_arr_sz;
        lmtf_arr = label_motif_count_[i];

        for (ui j = 0, s = 0; j < MOTIF_COUNT_DEMENSION; ++j)
        {
            pre_code_pre = (j << (LABEL_BIT_WIDTH * 2 + 1));
            //online:分别排序(code)放数组 //用lmtf_arr要填写的那段来做tmp
            ui b = s;
            for (auto item : cnt_map_arr[j])
            {
                pre_code = item.first;
                pre_code |= pre_code_pre;
                lmtf_arr[s].code = pre_code;
                lmtf_arr[s++].cnt = item.second;
            }

            std::sort(lmtf_arr + b, lmtf_arr + s, code_ascending);
        }
#if COLLECT_DATA_FEATURE == 0 //need one_cnt feature, so leave them
        //remove cnt==1 entries(only leave cnt>1)
        ui pos = 0;
        for (ui j = 0; j < lmtf_arr_sz; ++j)
        {
            if (lmtf_arr[j].cnt > 1)
            {
                lmtf_arr[pos].code = lmtf_arr[j].code;
                lmtf_arr[pos].cnt = lmtf_arr[j].cnt;
                ++pos;
            }
        }
        label_motif_count_sz_[i] = pos;
#else
        label_motif_count_sz_[i] = lmtf_arr_sz;
#endif //#if COLLECT_DATA_FEATURE == 0
#if WRITE_TO_FILE_DEBUG == 1
        fout << i << "\n";
        fout_codeInChar << "u" << i + 1 << "\n";
        for (ui j = 0; j < lmtf_arr_sz; ++j)
        {
            ui tmpcode = lmtf_arr[j].code;

            fout << tmpcode << spilt_c;

            ui d = tmpcode >> (2 * LABEL_BIT_WIDTH + 1);
            ui mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1
            ui lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
            ui lid2 = tmpcode & mask;
            fout_codeInChar
                << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
        }
        fout << "\n";
        fout_codeInChar << "\n";
        for (ui j = 0; j < lmtf_arr_sz; ++j)
        {
            fout << lmtf_arr[j].cnt << spilt_c;
            fout_codeInChar << lmtf_arr[j].cnt << spilt_c;
        }

        fout << "\n";
        fout_codeInChar << "\n";
#endif //WRITE_TO_FILE_DEBUG==1

        if (i && i % 100 == 0)
        {
            std::cout << "already finished motif count for " << i << " nodes" << std::endl;
        }
    }
    //write max_kind and min_kind
#if WRITE_TO_FILE_DEBUG == 1
    fout << "\n"
         << max_lmtf_kind_cnt << spilt_c << min_lmtf_kind_cnt;
    fout_codeInChar << "\n"
                    << max_lmtf_kind_cnt << spilt_c << min_lmtf_kind_cnt;
#endif //WRITE_TO_FILE_DEBUG==1

    /*
    RELEASE MEMORY AND CLOSE FILE
    */
    delete[] cnt_map_arr;

#endif //DIRECTED_GRAPH == 0

#if DIRECTED_GRAPH == 0
    fout.close();
#else
#if WRITE_TO_FILE_DEBUG == 1
    fout.close();
#endif //WRITE_TO_FILE_DEBUG==1
#endif //DIRECTED_GRAPH==0
#if WRITE_TO_FILE_DEBUG == 1
    fout_codeInChar.close();
#endif //WRITE_TO_FILE_DEBUG==1
}

#if ONLINE_STAGE == 0
//统计频率为1的特征占所有特征的占比
void Graph::generateMotifCount_label(ui &one_cnt, ui &total_cnt)
{
#if DIRECTED_GRAPH == 1
    std::map<ui, ui> *cnt_map_arr = new std::map<ui, ui>[MOTIF_COUNT_DEMENSION];
    Lmtf *lmtf_arr;

    for (ui i = 0; i < vertices_count_; ++i)
    {
        //i:vid
        directed_label_motif_count(i, cnt_map_arr);
        for (ui j = 0; j < MOTIF_COUNT_DEMENSION; ++j)
        {
            total_cnt += cnt_map_arr[j].size();
            for (auto it : cnt_map_arr[j])
            {
                if (it.first == 1)
                {
                    ++one_cnt;
                }
            }
        }
    }
    delete[] cnt_map_arr;

#endif //DIRECTED_GRAPH == 1
}
#endif //ONLINE_STAGE == 0

#endif //LABEL_MOTIF_ENABLE == 1

#if LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH == 1 && ONLINE_STAGE == 0
//label_directed_offline
//filename_pre:"xxx/youtube"，将把motif写入到xxx/youtube_directed_label_top_k.txt中，把min、max_lmtf_cnt写到xxx/youtube_directed_label_max_min.txt
void Graph::generateMotifCount_label(std::string filename_pre, ui vertexID_begin, ui vertex_period, ui kb, ui ke, std::string *kmethod_set, ui kmethod_num, bool wrtMaxMinKind2file)
{
    std::string filename_prefix;
    for (ui vertexID_b = vertexID_begin; vertexID_b < vertices_count_; vertexID_b += vertex_period)
    {
        /*
        OPEN FILE
        */
        filename_prefix = filename_pre;
        filename_prefix += "_directed_label_";
        if (kb > ke)
        {
            std::cout << "in generateMotifCount_label with DIRECTED_GRAPH==1 && ONLINE_STAGE==0:" << std::endl;
            std::cout << "kb>ke: kb<=ke is needed!" << std::endl;
            return;
        }

        ui ksz = (ke - kb + 1);
        std::ofstream *fout = new std::ofstream[ksz * kmethod_num]; //[k][method]
        std::string *name = kmethod_set;
        for (ui k = 0; k < ksz; ++k)
        {
            for (ui i = 0; i < kmethod_num; ++i)
            {
                std::string fname = filename_prefix + name[i] + "_" + std::to_string(k + kb) + ".txt";
                if (access(fname.c_str(), F_OK)) //file not exited->create
                {
                    FILE *fp = fopen(fname.c_str(), "a+");
                    if (fp)
                    {
                        fclose(fp);
                    }
                    else
                    {
                        std::cout << fname << " open/create failed! at kb=" << kb << ",ke=" << ke << std::endl;
                    }
                }
#ifdef DEBUG
                std::cout << fname << std::endl;
#endif //DEBUG
                fout[k * kmethod_num + i].open(fname, std::ios::app);
                if (!fout[k * kmethod_num + i].is_open())
                {
                    std::cout << "file not exited: " << filename_prefix + name[i] + ".txt" << std::endl;
                    return;
                }
            }
        }
#ifdef DEBUG
        std::ofstream *fout_codeInChar = new std::ofstream[ksz * kmethod_num]; //[k][method]
        for (ui k = 0; k < ksz; ++k)
        {
            for (ui i = 0; i < kmethod_num; ++i)
            {

                std::string fname = filename_prefix + "codeInChar_" + name[i] + "_" + std::to_string(k + kb) + ".txt";
                if (access(fname.c_str(), F_OK)) //file not exited->create
                {
                    FILE *fp = fopen(fname.c_str(), "a+");
                    if (fp)
                    {
                        fclose(fp);
                    }
                    else
                    {
                        std::cout << fname << " open/create failed! at kb=" << kb << ",ke=" << ke << std::endl;
                    }
                }
                //std::cout << filename_prefix + name[i] + "_" + std::to_string(k + kb) + ".txt" << std::endl;
                fout_codeInChar[k * kmethod_num + i].open(fname, std::ios::app);
                if (!fout_codeInChar[k * kmethod_num + i].is_open())
                {
                    std::cout << "file not exited: " << filename_prefix + name[i] + ".txt" << std::endl;
                    return;
                }
            }
        }
#endif //DEBUG

        /*
        COUNT MOTIF
        */
        std::map<ui, ui> *cnt_map_arr = new std::map<ui, ui>[MOTIF_COUNT_DEMENSION];
        Lmtf *lmtf_arr;
        lmtf_arr = new Lmtf[1];

        ui lmtf_arr_sz;
        ui pre_code, pre_code_pre, data_vertex;
        ui max_lmtf_kind_cnt = 0, min_lmtf_kind_cnt = -1;

        ui impossible_code = (1 << (2 * LABEL_BIT_WIDTH));
        Lmtf **lmtf_arr_G = new Lmtf *[kmethod_num]; //[method][k]
        for (ui i = 0; i < kmethod_num; ++i)
        {
            lmtf_arr_G[i] = new Lmtf[ke];
        }

        char spilt_c = ' ';
#ifdef DEBUG
        spilt_c = '\t';
#endif //
        ui vertices_count_end = std::min(vertexID_b + vertex_period, vertices_count_);
        for (ui i = vertexID_b; i < vertices_count_end; ++i)
        {
            //i:vid
            directed_label_motif_count(i, cnt_map_arr);
            /*
            offline: all turn struct arr and sort(descending cnt)
            */
            lmtf_arr_sz = 0;
            for (ui j = 0; j < MOTIF_COUNT_DEMENSION; ++j)
            {
                lmtf_arr_sz += cnt_map_arr[j].size();
            }

            if (wrtMaxMinKind2file)
            {
                if (lmtf_arr_sz > max_lmtf_kind_cnt)
                {
                    max_lmtf_kind_cnt = lmtf_arr_sz;
                }
                if (lmtf_arr_sz < min_lmtf_kind_cnt)
                {
                    min_lmtf_kind_cnt = lmtf_arr_sz;
                }
            }

            delete[] lmtf_arr;
            lmtf_arr = new Lmtf[lmtf_arr_sz];

            for (ui j = 0, s = 0; j < MOTIF_COUNT_DEMENSION; ++j)
            {
                pre_code_pre = (j << (LABEL_BIT_WIDTH * 2 + 1));
                for (auto item : cnt_map_arr[j])
                {
                    pre_code = item.first;
                    pre_code |= pre_code_pre;
                    lmtf_arr[s].code = pre_code;
                    lmtf_arr[s++].cnt = item.second;
                }
            }

            /*
            CHOOSE K AND WRITE
            */
            std::sort(lmtf_arr, lmtf_arr + lmtf_arr_sz, cnt_descending);
            for (ui k = kb; k <= ke; ++k)
            {
                int impossible_code_entry_num;
                choose_k_from_lmtf_arr(lmtf_arr, lmtf_arr_sz, k, lmtf_arr_G, kmethod_set, kmethod_num, impossible_code_entry_num);

                //配合loadmotifcountfromfile_label的加载算法，lmtf_arr_G中mid对应的那数组会原样加载到datagraph中
                /*write
                vertexID
                code_arr(code sequence not matter,k entries)
                cnt_arr(corresponding to code_arr)
                */
                for (ui j = 0; j < kmethod_num; ++j)
                {
                    ui fidx = (k - kb) * kmethod_num + j; //[k][method]
                    fout[fidx] << i << "\n";
#ifdef DEBUG
                    fout_codeInChar[fidx] << "v" << i + 1 << "\n";
#endif //DEBUG
                    if (kmethod_set[j] == "mid")
                    { //mid
                        int k0 = k - impossible_code_entry_num;
                        //code
                        //real feature segment
                        int e = k0 / 2;
                        for (int jj = 0; jj <= e; ++jj)
                        {
                            if (e - jj >= 0) //left first
                            {
                                ui tmpcode = lmtf_arr_G[j][e - jj].code; //[method][k]
#ifdef DEBUG
                                ui d = tmpcode >> (2 * LABEL_BIT_WIDTH + 1);
                                ui mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1
                                ui lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
                                ui lid2 = tmpcode & mask;
                                fout_codeInChar[fidx]
                                    << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
#endif //DEBUG
                                fout[fidx] << tmpcode << spilt_c;
                            }
                            if (jj && e + jj < k0) //right next; middle only write once
                            {
                                ui tmpcode = lmtf_arr_G[j][e + jj].code; //[method][k]
#ifdef DEBUG
                                ui d = tmpcode >> (2 * LABEL_BIT_WIDTH + 1);
                                ui mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1
                                ui lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
                                ui lid2 = tmpcode & mask;
                                fout_codeInChar[fidx]
                                    << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
#endif //DEBUG
                                fout[fidx] << tmpcode << spilt_c;
                            }
                        }

                        //对齐
                        for (int jj = k0; jj < k; ++jj)
                        {
                            ui tmpcode = (1 << (2 * LABEL_BIT_WIDTH)); //impossible_code
#ifdef DEBUG
                            ui d = tmpcode >> (2 * LABEL_BIT_WIDTH + 1);
                            ui mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1
                            ui lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
                            ui lid2 = tmpcode & mask;
                            fout_codeInChar[fidx]
                                << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
#endif //DEBUG
                            fout[fidx] << tmpcode << spilt_c;
                        }

                        fout[fidx] << "\n";
#ifdef DEBUG
                        fout_codeInChar[fidx] << "\n";
#endif //DEBUG \
    //cnt      \
    //real feature segment
                        for (int jj = 0; jj <= e; ++jj)
                        {
                            if (e - jj >= 0) //left first
                            {
                                fout[fidx] << lmtf_arr_G[j][e - jj].cnt << spilt_c;
#ifdef DEBUG
                                fout_codeInChar[fidx] << lmtf_arr_G[j][e - jj].cnt << spilt_c;
#endif //DEBUG
                            }
                            if (jj && e + jj < k0) //right next; middle only write once
                            {
                                fout[fidx] << lmtf_arr_G[j][e + jj].cnt << spilt_c;
#ifdef DEBUG
                                fout_codeInChar[fidx] << lmtf_arr_G[j][e + jj].cnt << spilt_c;
#endif //DEBUG
                            }
                        }

                        //对齐
                        for (int jj = k0; jj < k; ++jj)
                        {
                            fout[fidx] << 0 << spilt_c;
#ifdef DEBUG
                            fout_codeInChar[fidx] << 0 << spilt_c;
#endif //DEBUG
                        }
                    }
                    else
                    { //top down rand
                        for (ui jj = 0; jj < k; ++jj)
                        {
                            ui tmpcode = lmtf_arr_G[j][jj].code; //[method][k]
#ifdef DEBUG
                            ui d = tmpcode >> (2 * LABEL_BIT_WIDTH + 1);
                            ui mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1
                            ui lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
                            ui lid2 = tmpcode & mask;
                            fout_codeInChar[fidx]
                                << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
#endif //DEBUG
                            fout[fidx] << tmpcode << spilt_c;
                        }
                        fout[fidx] << "\n";
#ifdef DEBUG
                        fout_codeInChar[fidx] << "\n";
#endif //DEBUG
                        for (ui jj = 0; jj < k; ++jj)
                        {
                            fout[fidx] << lmtf_arr_G[j][jj].cnt << spilt_c;
#ifdef DEBUG
                            fout_codeInChar[fidx] << lmtf_arr_G[j][jj].cnt << spilt_c;
#endif //DEBUG
                        }
                    }
                    fout[fidx] << "\n";
#ifdef DEBUG
                    fout_codeInChar[fidx] << "\n";
#endif //DEBUG
                }
            }

            if (i && i % 100 == 0)
            {
                std::cout << "already finished motif count for " << i << " nodes" << std::endl;
            }
        }

        //write max_kind and min_kind
        if (wrtMaxMinKind2file)
        {
            std::ofstream fout_minmax(filename_prefix + "max_min_period.txt", std::ios::app);
            if (!fout_minmax.is_open())
            {
                std::cout << "file open fail: " << filename_prefix << "max_min_period.txt" << std::endl;
                return;
            }
            fout_minmax << vertexID_b << spilt_c << max_lmtf_kind_cnt << spilt_c << min_lmtf_kind_cnt << std::endl;
            fout_minmax.close();
        }
        /*
        RELEASE MEMORY AND CLOSE FILE
        */
        delete[] cnt_map_arr;
        for (ui i = 0; i < kmethod_num; ++i)
        {
            delete[] lmtf_arr_G[i];
        }
        delete[] lmtf_arr_G;
        delete[] lmtf_arr;

        for (ui i = 0; i < ksz * kmethod_num; ++i)
        {
            fout[i].close();
#ifdef DEBUG
            fout_codeInChar[i].close();
#endif //DEBUG
        }
        delete[] fout;
#ifdef DEBUG
        delete[] fout_codeInChar;
#endif //DEBUG
    }
    //write max_kind and min_kind
    if (wrtMaxMinKind2file)
    {
        std::ifstream fin(filename_pre + "_directed_label_max_min_period.txt");
        std::ofstream fout_minmax(filename_pre + "_directed_label_max_min.txt");
        ui vid, minv, maxv;
        ui max_kind = 0, min_kind = -1; //-1:on purpose
        while (fin >> vid >> maxv >> minv)
        {
            if (minv < min_kind)
            {
                min_kind = minv;
            }
            if (maxv > max_kind)
            {
                max_kind = maxv;
            }
        }
        fout_minmax << max_kind << " " << min_kind;
        fout_minmax.close();
        fin.close();
    }
}
#endif //LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH == 1 && ONLINE_STAGE == 0

#if LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH == 1 && LABEL_MOTIF_LIMIT == 1
//filename_prefix:"xxx/youtube"
//online：不写；如果WRITE_TO_FILE_DEBUG==1则把CodeInChar结果写入xxx/youtube_directed_label_limit_codeInChar.txt中
//offline：把file format结果写入到xxx/youtube_directed_label_limit.txt中
/*
file format:
vid code_cnt
code cnt code cnt……

CodeInChar file format:
u(vid+1): code_cnt
CodeInChar cnt  (vx,vy) (vi,vj)...
CodeInChar cnt ...
*/
void Graph::generateMotifCount_label_limit(std::string filename_prefix)
{
    /*
    OPEN FILE
    */
    filename_prefix += "_directed_label_limit"; //xxx/youtube_directed_label_limit
#if ONLINE_STAGE == 0
    std::ofstream fout(filename_prefix + ".txt");
    if (!fout.is_open())
    {
        std::cout << "file not exited: " << filename_prefix + ".txt" << std::endl;
        return;
    }
#endif //#if ONLINE_STAGE == 0
#if WRITE_TO_FILE_DEBUG == 1
    std::ofstream fout_codeInChar(filename_prefix + "_codeInChar.txt");
    if (!fout_codeInChar.is_open())
    {
        std::cout << "file not exited: " << filename_prefix + "_codeInChar.txt" << std::endl;
        return;
    }
#endif //WRITE_TO_FILE_DEBUG==1

    /*
    COUNT MOTIF
    */

    std::map<ui, ui> *one_vertex_cnt_map; //map for one vertex

#if ONLINE_STAGE == 1
    //alloc
    label_motif_map_ = new std::map<ui, ui>[vertices_count_];
#else
    one_vertex_cnt_map = new std::map<ui, ui>;
#endif //#if ONLINE_STAGE == 1

    char spilt_c = ' ';
#if WRITE_TO_FILE_DEBUG == 1
    spilt_c = '\t';
    std::map<ui, std::vector<ui>> *code2vertexvertex = new std::map<ui, std::vector<ui>>;
#endif //WRITE_TO_FILE_DEBUG==1
    for (ui i = 0; i < vertices_count_; ++i)
    {
//i:vid
#if ONLINE_STAGE == 0
        one_vertex_cnt_map->clear();
#else  //ONLINE_STAGE==1:
        one_vertex_cnt_map = label_motif_map_ + i;
#endif //ONLINE_STAGE==0

#if WRITE_TO_FILE_DEBUG == 0
        directed_label_motif_count_limit(i, one_vertex_cnt_map);
#else
        code2vertexvertex->clear();
        directed_label_motif_count_limit_debug(i, one_vertex_cnt_map, code2vertexvertex);
#endif //#if WRITE_TO_FILE_DEBUG == 0

#if ONLINE_STAGE == 0
        fout << i << spilt_c << one_vertex_cnt_map->size() << "\n";
        for (auto item : *one_vertex_cnt_map)
        {
            fout << item.first << spilt_c << item.second << spilt_c;
        }
        fout << "\n";
#endif //#if ONLINE_STAGE == 0
#if WRITE_TO_FILE_DEBUG == 1
        fout_codeInChar << "v" << i + 1 << spilt_c << one_vertex_cnt_map->size() << "\n";
        //codeinchar
        for (auto item : *one_vertex_cnt_map)
        {
            ui tmpcode = item.first;
            ui d = tmpcode >> LABEL_SEG_WIDTH;
            ui mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1其他全0
            ui lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
            ui lid2 = tmpcode & mask;
            fout_codeInChar
                << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
            fout_codeInChar << item.second; //<< "\n";

            for (auto cd : (*code2vertexvertex)[tmpcode])
            {
                fout_codeInChar << spilt_c << "(v" << (cd >> 16) + 1 << ",v" << (cd & 0xffff) + 1 << ")";
            }
            fout_codeInChar << "\n";
        }
        fout_codeInChar << "\n";
#endif //WRITE_TO_FILE_DEBUG==1

        if (i && i % 100 == 0)
        {
            std::cout << "already finished motif count for " << i << " nodes" << std::endl;
        }
    }

    /*
    RELEASE MEMORY AND CLOSE FILE
    */

#if ONLINE_STAGE == 0
    fout.close();
    delete one_vertex_cnt_map;
#endif //ONLINE_STAGE == 0

#if WRITE_TO_FILE_DEBUG == 1
    fout_codeInChar.close();
    delete code2vertexvertex;
#endif //WRITE_TO_FILE_DEBUG==1
}
#endif //#if LABEL_MOTIF_ENABLE == 1 && DIRECTED_GRAPH==1 && LABEL_MOTIF_LIMIT==1

/*============================
LOAD MOTIF FROM FILE
*/
#if DIRECTED_GRAPH == 0 && LABEL_MOTIF_ENABLE == 1
//filename_prefix:""xxx/youtube"，将从xxx/youtube_undirected_label.txt中加载motif_struct
void Graph::loadMotifCountFromFile_label(std::string filename_prefix)
{
}
#endif //DIRECTED_GRAPH == 0 && LABEL_MOTIF_ENABLE == 1

#if DIRECTED_GRAPH == 1 && LABEL_MOTIF_ENABLE == 1 //only for data_graph
/*
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
void Graph::loadMotifCountFromFile_label(std::string filename_prefix, ui kInFileName, std::string kmethod_name, ui ko)
{
    k = ko;

    filename_prefix += "_directed_label_" + kmethod_name + "_" + std::to_string(kInFileName) + ".txt";
#ifdef DEBUG
    std::cout << "load motif count for data_graph from file:" << filename_prefix << std::endl;
#endif //DEBUG

    std::ifstream fin(filename_prefix);
    //allocate //G:不用存label_motif_count_sz_
    label_motif_count_ = new Lmtf *[vertices_count_];
    ui vid;
    std::string following_in_line;
    for (ui i = 0; i < vertices_count_; ++i)
    {
        label_motif_count_[i] = new Lmtf[k];
        fin >> vid;
        ui e = k;
        ui tmpcnt;
        if (kmethod_name == "mid")
        { // for code ascending in G_motif_arr
            ui mid = k / 2;
            for (ui j = 0; j <= mid; ++j)
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
            for (ui j = 0; j <= mid; ++j)
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
            for (ui j = 0; j < k; ++j)
            {
                fin >> label_motif_count_[i][j].code;
            }
            std::getline(fin, following_in_line); //absorb the rest of this line
            for (ui j = 0; j < k; ++j)
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
}

#if LABEL_MOTIF_LIMIT == 1
//filename_prefix:"xxx/youtube",xxx/youtube_directed_label_limit.txt中加载
//在这里分配空间
/*
file format:
vid code_cnt
code cnt code cnt……
*/
void Graph::loadMotifCountFromFile_label_limit(std::string filename_prefix)
{
    label_motif_map_ = new std::map<ui, ui>[vertices_count_];
    ui vid, code_cnt, code, cnt;
    std::ifstream fin(filename_prefix + "_directed_label_limit.txt");
    std::map<ui, ui> *one_vertex_cnt_map;
    while (fin >> vid >> code_cnt)
    {
        one_vertex_cnt_map = label_motif_map_ + vid;
        for (ui i = 0; i < code_cnt; ++i)
        {
            fin >> code >> cnt;
            (*one_vertex_cnt_map)[code] = cnt;
        }
    }
    fin.close();
}
#endif //LABEL_MOTIF_LIMIT
#endif //#if DIRECTED_GRAPH == 1 && LABEL_MOTIF_ENABLE==1

#if TOPO_MOTIF_ENABLE == 1
//file format: VertexID MOTIF_COUNT_DEMENSIONcount
//filename_prefix:"xxx/youtube",将从xxx/youtube_(un)directed.txt中加载motif_struct
void Graph::loadMotifCountFromFile(std::string filename_prefix)
{
#if DIRECTED_GRAPH == 1
    filename_prefix += "_directed";
#else
    filename_prefix += "_undirected";
#endif // DIRECTED_GRAPH
    std::ifstream fin(filename_prefix + ".txt");
    if (!fin.is_open())
    {
        std::cout << "file not exited: " << filename_prefix + ".txt" << std::endl;
        return;
    }
#if DIRECTED_GRAPH == 1
    motif_count_ = new ui *[vertices_count_];
    ui i;
    for (i = 0; i < vertices_count_; ++i)
    {
        motif_count_[i] = new ui[MOTIF_COUNT_DEMENSION];
    }
    ui *motif_cnt;
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
#else
    tri_count_ = new ui[vertices_count_];
    std::string line;
    VertexID vid;
    while (getline(fin, line))
    {
        std::stringstream ss(line);
        ss >> vid >> tri_count_[vid];
    }
#endif // DIRECTED_GRAPH

    fin.close();
}
#endif //TOPO_MOTIF_ENABLE==1

/*
    label print as char: labelID+'A'
    vid print as 'v'vid+1
    - label->vertices
    directed:
    - each vertex: in out bi neighbors;
    - each vertex: in out bi nlf_struct(in egonetwork:each label freq);
    */
void Graph::print_graph_detail()
{
    std::cout << "label->vid:" << std::endl;
    for (ui i = 0; i < labels_count_; ++i)
    {
        std::cout << char(i + 'A') << ": ";
        ui cnt;
        const ui *arr = getVerticesByLabel(i, cnt);
        for (ui j = 0; j < cnt; ++j)
        {
            std::cout << "v" << arr[j] + 1 << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "-----" << std::endl;
    std::cout << std::endl;
    std::cout << "neighbors and nlf:" << std::endl;
    for (ui i = 0; i < vertices_count_; ++i)
    {
        std::cout << "v" << i + 1 << ":" << std::endl;
        std::cout << "in neighbors: ";
        ui cnt;
        const ui *arr = getVertexInNeighbors(i, cnt);
        for (ui j = 0; j < cnt; ++j)
        {
            std::cout << "v" << arr[j] + 1 << " ";
        }
        std::cout << std::endl;
        std::cout << "in nlf: " << std::endl;
        const std::unordered_map<LabelID, ui> *pmap = getVertexInNLF(i);
        for (auto it : *pmap)
        {
            std::cout << char(it.first + 'A') << ":" << it.second << " ";
        }
        std::cout << std::endl;

        std::cout << "out neighbors: ";
        arr = getVertexOutNeighbors(i, cnt);
        for (ui j = 0; j < cnt; ++j)
        {
            std::cout << "v" << arr[j] + 1 << " ";
        }
        std::cout << std::endl;
        std::cout << "out nlf: " << std::endl;
        pmap = getVertexOutNLF(i);
        for (auto it : *pmap)
        {
            std::cout << char(it.first + 'A') << ":" << it.second << " ";
        }
        std::cout << std::endl;

        std::cout << "bi neighbors: ";
        arr = getVertexBiNeighbors(i, cnt);
        for (ui j = 0; j < cnt; ++j)
        {
            std::cout << "v" << arr[j] + 1 << " ";
        }
        std::cout << std::endl;
        std::cout << "bi nlf: " << std::endl;
        pmap = getVertexBiNLF(i);
        for (auto it : *pmap)
        {
            std::cout << char(it.first + 'A') << ":" << it.second << " ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
}

#if LABEL_MOTIF_ENABLE == 1
// use_mtf_sz_cnt为1：标签有向图会使用label_motif_count_sz_数组
void Graph::print_graph_mtf(bool use_mtf_sz_cnt)
{
    char spilt_c = '\t';
    if (use_mtf_sz_cnt)
    {
        ui dime;
        for (ui i = 0; i < vertices_count_; ++i)
        {
            std::cout << "v" << i + 1 << ":" << std::endl;
            Lmtf *arr = getVertexLabelMotifCount(i, dime);
            for (ui j = 0; j < dime; ++j)
            {
                ui tmpcode = arr[j].code;
                ui d = tmpcode >> (2 * LABEL_BIT_WIDTH + 1);
                ui mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1
                ui lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
                ui lid2 = tmpcode & mask;
                std::cout
                    << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
            }
            std::cout << std::endl;
            for (ui j = 0; j < dime; ++j)
            {
                std::cout << arr[j].cnt << spilt_c;
            }
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "k:" << k << std::endl;
        for (ui i = 0; i < vertices_count_; ++i)
        {
            std::cout << "v" << i + 1 << ":" << std::endl;
            Lmtf *arr = getVertexLabelMotifCount(i);
            for (ui j = 0; j < k; ++j)
            {
                ui tmpcode = arr[j].code;
                ui d = tmpcode >> (2 * LABEL_BIT_WIDTH + 1);
                ui mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1
                ui lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
                ui lid2 = tmpcode & mask;
                std::cout
                    << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
            }
            std::cout << std::endl;
            for (ui j = 0; j < k; ++j)
            {
                std::cout << arr[j].cnt << std::endl;
            }
            std::cout << std::endl;
        }
    }
}
#if LABEL_MOTIF_LIMIT == 1
void Graph::print_label_motif_map()
{
    std::map<ui, ui> *pmap;
    for (ui i = 0; i < vertices_count_; ++i)
    {
        std::cout << "v" << i + 1 << ":\n";
        pmap = label_motif_map_ + i;
        ui mask = (1 << LABEL_BIT_WIDTH) - 1;
        for (auto item : *pmap)
        {
            ui code = item.first;
            std::cout << (code >> LABEL_SEG_WIDTH) << "_" << char(((code >> LABEL_BIT_WIDTH) & mask) + 'A') << "_" << char((code & mask) + 'A') << " ";
            
std::cout << item.second << "\n";
        }
        std::cout << "\n";
    }
}
#endif //LABEL_MOTIF_LIMIT==1

#endif //LABEL_MOTIF_ENABLE==1

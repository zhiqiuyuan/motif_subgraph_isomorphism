#include "Motif.h"

Motif::Motif() : Graph()
{
#if TOPO_MOTIF_ENABLE == 1
    motif_count_ = NULL;
#endif //TOPO_MOTIF_ENABLE==1

#if LABEL_MOTIF_ENABLE == 1
    label_motif_count_ = NULL;
    label_motif_count_sz_ = NULL;
    in_nlf_ = NULL;
    out_nlf_ = NULL;
    bi_nlf_ = NULL;
#if LABEL_MOTIF_LIMIT == 1
    label_motif_map_ = NULL;
#endif //LABEL_MOTIF_LIMIT==1

#endif //LABEL_MOTIF_ENABLE==1
    reverse_index_offsets_ = NULL;
    reverse_index_ = NULL;
}

Motif::~Motif()
{
#if TOPO_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        delete[] motif_count_[i];
    }
    delete[] motif_count_;
#endif //TOPO_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1

#if LABEL_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
    delete[] in_nlf_;
    delete[] out_nlf_;
    delete[] bi_nlf_;
#if LABEL_MOTIF_LIMIT == 0
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        delete[] label_motif_count_[i];
    }
    delete[] label_motif_count_;
    delete[] label_motif_count_sz_;
#else  //LABEL_MOTIF_LIMIT == 1:
    delete[] label_motif_map_;
#endif //LABEL_MOTIF_LIMIT == 0

#endif //LABEL_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1
    delete[] reverse_index_offsets_;
    delete[] reverse_index_;
}

/*
 * TOPO_MOTIF_ENABLE
 */
#if TOPO_MOTIF_ENABLE == 1
bool Motif::generateTopoMotifCount(std::string filename_prefix)
{
#if (ONLINE_STAGE == 0 || WRITE_TO_FILE_DEBUG == 1)
    if (filename_prefix == "")
    {
        std::cout << __FILE__ << ": " << __LINE__;
        std::cout << "\n\tin generateTopoMotifCount: no filename_prefix specified, return\n";
        return 0;
    }
#endif                          //(ONLINE_STAGE == 0 || WRITE_TO_FILE_DEBUG == 1)
    filename_prefix += "_topo"; //xxx/youtube_topo
#if ONLINE_STAGE == 0
    std::ofstream fout(filename_prefix + ".txt");
    if (!fout.is_open())
    {
        std::cout << __FILE__ << ": " << __LINE__;
        std::cout << "\n\tfile not exited : " << filename_prefix + ".txt " << std::endl;
        return 0;
    }
#endif //ONLINE_STAGE == 0
#if WRITE_TO_FILE_DEBUG == 1
    std::ofstream fout_debug(filename_prefix + "_debug.txt");
    if (!fout_debug.is_open())
    {
        std::cout << __FILE__ << ": " << __LINE__;
        std::cout << "\n\tfile not exited: " << filename_prefix + ".txt" << std::endl;
        return 0;
    }
#endif //#if WRITE_TO_FILE_DEBUG == 1

    unsigned *motif_cnt;
#if ONLINE_STAGE == 1
    motif_count_ = new unsigned *[vertices_count_];
    for (size_t i = 0; i < vertices_count_; ++i)
    {
        motif_count_[i] = new unsigned[MOTIF_COUNT_DEMENSION]();
    }
#else  //ONLINE_STAGE==0
    motif_cnt = new unsigned[MOTIF_COUNT_DEMENSION]();
#endif // ONLINE_STAGE==1

    for (unsigned i = 0; i < vertices_count_; ++i)
    { //i:vid
#if ONLINE_STAGE == 1
        motif_cnt = motif_count_[i];
#else
        memset(motif_cnt, 0, sizeof(unsigned) * MOTIF_COUNT_DEMENSION);
#endif //ONLINE_STAGE==1
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
        for (unsigned a = 0; a < MOTIF_COUNT_DEMENSION; ++a)
        {
            fout << split_c << motif_cnt[a];
        }
        fout << "\n";
#endif //#if ONLINE_STAGE == 0
#if WRITE_TO_FILE_DEBUG == 1
        split_c = '\t';
        fout_debug << "v" << i + 1 << "\n";
        for (unsigned a = 0; a < MOTIF_COUNT_DEMENSION; ++a)
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
#if RUNNING_COMMENT == 1
        if (i % 100 == 0)
        {
            std::cout << "already finished motif count for " << i << " nodes" << std::endl;
        }
#endif //RUNNING_COMMENT==1
    }
#if ONLINE_STAGE == 0
    delete[] motif_cnt;
    fout.close();
#endif //#if ONLINE_STAGE == 0
#if WRITE_TO_FILE_DEBUG == 1
    fout_debug.close();
#endif //#if WRITE_TO_FILE_DEBUG == 1
    return 1;
}

void Motif::directed_motif_count(unsigned i, unsigned *motif_cnt)
{
    const unsigned *in, *out, *bi;
    unsigned in_count, out_count, bi_count;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    //out out
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = a + 1; b < out_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = out[b];
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = 0; b < in_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = in[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
                if (ab)
                {
                    motif_cnt[5]++;
                }
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = 0; b < bi_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = bi[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
                if (ab)
                {
                    motif_cnt[7]++;
                }
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < in_count; ++a)
    {
        for (unsigned b = a + 1; b < in_count; ++b)
        {
            unsigned va = in[a];
            unsigned vb = in[b];
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < in_count; ++a)
    {
        for (unsigned b = 0; b < bi_count; ++b)
        {
            unsigned va = in[a];
            unsigned vb = bi[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
                if (ab)
                {
                    motif_cnt[9]++;
                }
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < bi_count; ++a)
    {
        for (unsigned b = a + 1; b < bi_count; ++b)
        {
            unsigned va = bi[a];
            unsigned vb = bi[b];
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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

void Motif::directed_motif_count_debug(unsigned i, unsigned *motif_cnt, std::vector<std::vector<std::pair<int, int>>> &motif_detail)
{
    const unsigned *in, *out, *bi;
    unsigned in_count, out_count, bi_count;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    //out out
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = a + 1; b < out_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = out[b];
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = 0; b < in_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = in[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
                if (ab)
                {
                    motif_cnt[5]++;
                    motif_detail[5].push_back(std::pair<int, int>(va, vb));
                }
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = 0; b < bi_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = bi[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
                if (ab)
                {
                    motif_cnt[7]++;
                    motif_detail[7].push_back(std::pair<int, int>(va, vb));
                }
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < in_count; ++a)
    {
        for (unsigned b = a + 1; b < in_count; ++b)
        {
            unsigned va = in[a];
            unsigned vb = in[b];
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < in_count; ++a)
    {
        for (unsigned b = 0; b < bi_count; ++b)
        {
            unsigned va = in[a];
            unsigned vb = bi[b];
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
                if (ab)
                {
                    motif_cnt[9]++;
                    motif_detail[9].push_back(std::pair<int, int>(va, vb));
                }
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < bi_count; ++a)
    {
        for (unsigned b = a + 1; b < bi_count; ++b)
        {
            unsigned va = bi[a];
            unsigned vb = bi[b];
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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

#endif //TOPO_MOTIF_ENABLE==1

/*
 * LABEL_MOTIF_ENABLE
 */
#if LABEL_MOTIF_ENABLE == 1

bool Motif::generateLabelMotifCount(std::string filename_prefix)
{
#if (ONLINE_STAGE == 0 || WRITE_TO_FILE_DEBUG == 1)
    if (filename_prefix == "")
    {
        std::cout << __FILE__ << ": " << __LINE__;
        std::cout << "\n\tin generateLabelMotifCount: no filename_prefix specified, return\n";
        return 0;
    }
#endif                           //(ONLINE_STAGE == 0 || WRITE_TO_FILE_DEBUG == 1)
    filename_prefix += "_label"; //xxx/youtube_label
#if WRITE_TO_FILE_DEBUG == 1
    std::ofstream fout(filename_prefix + ".txt");
    if (!fout.is_open())
    {
        std::cout << __FILE__ << ": " << __LINE__;
        std::cout << "\n\tfile not exited : " << filename_prefix + ".txt " << std::endl;
        return 0;
    }

    std::ofstream fout_codeInChar(filename_prefix + "_codeInChar.txt");
    if (!fout_codeInChar.is_open())
    {
        std::cout << __FILE__ << ": " << __LINE__;
        std::cout << "\n\tfile not exited : " << filename_prefix + ".txt " << std::endl;
        return 0;
    }
#endif //WRITE_TO_FILE_DEBUG==1

    /*
    COUNT MOTIF
    */
    std::map<unsigned, unsigned> *cnt_map_arr = new std::map<unsigned, unsigned>[MOTIF_COUNT_DEMENSION];
    Lmtf *lmtf_arr;
    label_motif_count_ = new Lmtf *[vertices_count_];
    label_motif_count_sz_ = new unsigned[vertices_count_];
    unsigned lmtf_arr_sz;
    unsigned pre_code, pre_code_pre, data_vertex;
    unsigned max_lmtf_kind_cnt = 0, min_lmtf_kind_cnt = -1;

    char spilt_c = ' ';
#if WRITE_TO_FILE_DEBUG == 1
    spilt_c = '\t';
#endif //WRITE_TO_FILE_DEBUG==1
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        //i:vid
        directed_label_motif_count(i, cnt_map_arr);
        /*
        online: turn struct arr and sort(ascending code) respectively
        */
        lmtf_arr_sz = 0;
        for (unsigned j = 0; j < MOTIF_COUNT_DEMENSION; ++j)
        {
            lmtf_arr_sz += cnt_map_arr[j].size();
        }
#if WRITE_TO_FILE_DEBUG == 1
        if (lmtf_arr_sz > max_lmtf_kind_cnt)
        {
            max_lmtf_kind_cnt = lmtf_arr_sz;
        }
        if (lmtf_arr_sz < min_lmtf_kind_cnt)
        {
            min_lmtf_kind_cnt = lmtf_arr_sz;
        }
#endif //#if WRITE_TO_FILE_DEBUG == 1

        label_motif_count_[i] = new Lmtf[lmtf_arr_sz];
        //label_motif_count_sz_[i] = lmtf_arr_sz;
        lmtf_arr = label_motif_count_[i];

        for (unsigned j = 0, s = 0; j < MOTIF_COUNT_DEMENSION; ++j)
        {
            pre_code_pre = (j << (LABEL_BIT_WIDTH * 2 + 1));
            //online:分别排序(code)放数组 //用lmtf_arr要填写的那段来做tmp
            unsigned b = s;
            for (auto item : cnt_map_arr[j])
            {
                pre_code = item.first;
                pre_code |= pre_code_pre;
                lmtf_arr[s].code = pre_code;
                lmtf_arr[s++].cnt = item.second;
            }

            std::sort(lmtf_arr + b, lmtf_arr + s, code_ascending);
        }
        //如果下面注释的部分取消注释（即去除频率为1的特征），则这行要注释
        label_motif_count_sz_[i] = lmtf_arr_sz;
        /*
        //remove cnt==1 entries(only leave cnt>1)
        unsigned pos = 0;
        for (unsigned j = 0; j < lmtf_arr_sz; ++j)
        {
            if (lmtf_arr[j].cnt > 1)
            {
                lmtf_arr[pos].code = lmtf_arr[j].code;
                lmtf_arr[pos].cnt = lmtf_arr[j].cnt;
                ++pos;
            }
        }
        label_motif_count_sz_[i] = pos;
        */

#if WRITE_TO_FILE_DEBUG == 1
        fout << i << "\n";
        fout_codeInChar << "u" << i + 1 << "\n";
        for (unsigned j = 0; j < lmtf_arr_sz; ++j)
        {
            unsigned tmpcode = lmtf_arr[j].code;

            fout << tmpcode << spilt_c;

            unsigned d = tmpcode >> (2 * LABEL_BIT_WIDTH + 1);
            unsigned mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1
            unsigned lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
            unsigned lid2 = tmpcode & mask;
            fout_codeInChar
                << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
        }
        fout << "\n";
        fout_codeInChar << "\n";
        for (unsigned j = 0; j < lmtf_arr_sz; ++j)
        {
            fout << lmtf_arr[j].cnt << spilt_c;
            fout_codeInChar << lmtf_arr[j].cnt << spilt_c;
        }

        fout << "\n";
        fout_codeInChar << "\n";
#endif //WRITE_TO_FILE_DEBUG==1
#if RUNNING_COMMENT == 1
        if (i && i % 100 == 0)
        {
            std::cout << "already finished motif count for " << i << " nodes" << std::endl;
        }
#endif //RUNNING_COMMENT
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

#if WRITE_TO_FILE_DEBUG == 1
    fout.close();
#endif //WRITE_TO_FILE_DEBUG==1
#if WRITE_TO_FILE_DEBUG == 1
    fout_codeInChar.close();
#endif //WRITE_TO_FILE_DEBUG==1
    return 1;
}

void Motif::BuildNLF()
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

#if LABEL_MOTIF_LIMIT == 1
/* filename_prefix:xxx/youtube
* 覆盖写
* 离线：数所有特征，以file format排版写入xxx/youtube_label_limit.txt；
*       如果WRITE_TO_FILE_DEBUG则结果也会写入xxx/youtube_label_limit_codeInChar.txt
* 在线：不写文件，给申请空间；
*       数所有特征存std::map<unsigned, unsigned> *label_motif_map_中；
*       如果WRITE_TO_FILE_DEBUG则结果会写入xxx/youtube_label_limit_codeInChar.txt，并统计最大最小特征种类数（写入文件尾）
*/
/*
file format:
vid code_cnt
code cnt code cnt……

CodeInChar file format:
u(vid+1): code_cnt
CodeInChar cnt  (vx,vy) (vi,vj)...
CodeInChar cnt ...
*/
bool Motif::generateLabelMotifCount_limit(std::string filename_prefix)
{
#if (ONLINE_STAGE == 0 || WRITE_TO_FILE_DEBUG == 1)
    if (filename_prefix == "")
    {
        std::cout << __FILE__ << ": " << __LINE__;
        std::cout << "\n\tin generateLabelMotifCount_limit: no filename_prefix specified, return\n";
        return 0;
    }
#endif                                 //(ONLINE_STAGE == 0 || WRITE_TO_FILE_DEBUG == 1)
    filename_prefix += "_label_limit"; //xxx/youtube_label_limit
#if ONLINE_STAGE == 0
    std::ofstream fout(filename_prefix + ".txt");
    if (!fout.is_open())
    {
        std::cout << __FILE__ << ": " << __LINE__;
        std::cout << "\n\tfile not exited : " << filename_prefix + ".txt " << std::endl;
        return 0;
    }
#endif //#if ONLINE_STAGE == 0
#if WRITE_TO_FILE_DEBUG == 1
    std::ofstream fout_codeInChar(filename_prefix + "_codeInChar.txt");
    if (!fout_codeInChar.is_open())
    {
        std::cout << __FILE__ << ": " << __LINE__;
        std::cout << "\n\tfile not exited : " << filename_prefix + ".txt " << std::endl;
        return 0;
    }
#endif //WRITE_TO_FILE_DEBUG==1

    /*
    COUNT MOTIF
    */
    std::map<unsigned, unsigned> *one_vertex_cnt_map; //map for one vertex

#if ONLINE_STAGE == 1
    //alloc
    label_motif_map_ = new std::map<unsigned, unsigned>[vertices_count_];
#else
    one_vertex_cnt_map = new std::map<unsigned, unsigned>;
#endif //#if ONLINE_STAGE == 1

    char spilt_c = ' ';
#if WRITE_TO_FILE_DEBUG == 1
    spilt_c = '\t';
    std::map<unsigned, std::vector<unsigned>> *code2vertexvertex = new std::map<unsigned, std::vector<unsigned>>;
#endif //WRITE_TO_FILE_DEBUG==1
    for (unsigned i = 0; i < vertices_count_; ++i)
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
            unsigned tmpcode = item.first;
            unsigned d = tmpcode >> LABEL_SEG_WIDTH;
            unsigned mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1其他全0
            unsigned lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
            unsigned lid2 = tmpcode & mask;
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
#if RUNNING_COMMENT == 1
        if (i && i % 100 == 0)
        {
            std::cout << "already finished motif count for " << i << " nodes" << std::endl;
        }
#endif //RUNNING_COMMENT==1
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
    return 1;
}
#endif //LABEL_MOTIF_LIMIT==1

/* 顶点i的所有特征都写入cnt_map_arr
* for vertex i
* value in cnt_map_arr needs to be already specified
* memory it points to needs to be already allocated
*/
void Motif::directed_label_motif_count(unsigned i, std::map<unsigned, unsigned> *cnt_map_arr)
{
    const unsigned *in, *out, *bi, *_in, *_out, *_bi;
    unsigned in_count, out_count, bi_count, _in_count, _out_count, _bi_count;
    unsigned pre_code, pre_code_pre, data_vertex;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    for (unsigned j = 0; j < MOTIF_COUNT_DEMENSION; ++j)
    {
        cnt_map_arr[j].clear();
    }
    //in
    for (unsigned _ = 0; _ < in_count; ++_)
    {
        data_vertex = in[_];
        pre_code_pre = (getVertexLabel(data_vertex) << LABEL_BIT_WIDTH);
        _in = getVertexInNeighbors(data_vertex, _in_count);
        _out = getVertexOutNeighbors(data_vertex, _out_count);
        _bi = getVertexBiNeighbors(data_vertex, _bi_count);
        //in
        for (unsigned a = 0; a < _in_count; ++a)
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
            if (checkEdgeExistence(data_vertex, i))
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
        for (unsigned a = 0; a < _out_count; ++a)
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
            if (checkEdgeExistence(i, data_vertex))
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
        for (unsigned a = 0; a < _bi_count; ++a)
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
            if (checkEdgeExistence(data_vertex, i))
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
    for (unsigned _ = 0; _ < out_count; ++_)
    {
        data_vertex = out[_];
        pre_code_pre = (getVertexLabel(data_vertex) << LABEL_BIT_WIDTH);
        _in = getVertexInNeighbors(data_vertex, _in_count);
        _out = getVertexOutNeighbors(data_vertex, _out_count);
        _bi = getVertexBiNeighbors(data_vertex, _bi_count);
        //in
        for (unsigned a = 0; a < _in_count; ++a)
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
            if (checkEdgeExistence(i, data_vertex))
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
        for (unsigned a = 0; a < _out_count; ++a)
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
            if (checkEdgeExistence(data_vertex, i))
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
        for (unsigned a = 0; a < _bi_count; ++a)
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
            if (checkEdgeExistence(i, data_vertex))
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
            if (checkEdgeExistence(data_vertex, i))
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
    for (unsigned _ = 0; _ < bi_count; ++_)
    {
        data_vertex = bi[_];
        pre_code_pre = (getVertexLabel(data_vertex) << LABEL_BIT_WIDTH);
        _in = getVertexInNeighbors(data_vertex, _in_count);
        _out = getVertexOutNeighbors(data_vertex, _out_count);
        _bi = getVertexBiNeighbors(data_vertex, _bi_count);
        //in
        for (unsigned a = 0; a < _in_count; ++a)
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
            if (checkEdgeExistence(data_vertex, i))
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
        for (unsigned a = 0; a < _out_count; ++a)
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
            if (checkEdgeExistence(i, data_vertex))
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
            if (checkEdgeExistence(data_vertex, i))
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
        for (unsigned a = 0; a < _bi_count; ++a)
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
            if (checkEdgeExistence(i, data_vertex))
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

#if LABEL_MOTIF_LIMIT == 1
void Motif::directed_label_motif_count_limit(unsigned i, std::map<unsigned, unsigned> *one_vertex_cnt_map)
{
    const unsigned *in, *out, *bi;
    unsigned in_count, out_count, bi_count;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    unsigned code, label_code_seg;

    //out out
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = a + 1; b < out_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = out[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
                    unsigned tmp = alb;
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
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = 0; b < in_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = in[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
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
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = 0; b < bi_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = bi[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
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
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < in_count; ++a)
    {
        for (unsigned b = a + 1; b < in_count; ++b)
        {
            unsigned va = in[a];
            unsigned vb = in[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
                    unsigned tmp = alb;
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
    for (unsigned a = 0; a < in_count; ++a)
    {
        for (unsigned b = 0; b < bi_count; ++b)
        {
            unsigned va = in[a];
            unsigned vb = bi[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
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
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < bi_count; ++a)
    {
        for (unsigned b = a + 1; b < bi_count; ++b)
        {
            unsigned va = bi[a];
            unsigned vb = bi[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
                    unsigned tmp = alb;
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

void Motif::directed_label_motif_count_limit_debug(unsigned i, std::map<unsigned, unsigned> *one_vertex_cnt_map, std::map<unsigned, std::vector<unsigned>> *code2vertexvertex)
{
    const unsigned *in, *out, *bi;
    unsigned in_count, out_count, bi_count;

    in = getVertexInNeighbors(i, in_count);
    out = getVertexOutNeighbors(i, out_count);
    bi = getVertexBiNeighbors(i, bi_count);

    unsigned code, label_code_seg;
    //out out
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = a + 1; b < out_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = out[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
                    unsigned tmp = alb;
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
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = 0; b < in_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = in[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
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
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < out_count; ++a)
    {
        for (unsigned b = 0; b < bi_count; ++b)
        {
            unsigned va = out[a];
            unsigned vb = bi[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
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
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < in_count; ++a)
    {
        for (unsigned b = a + 1; b < in_count; ++b)
        {
            unsigned va = in[a];
            unsigned vb = in[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
                    unsigned tmp = alb;
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
    for (unsigned a = 0; a < in_count; ++a)
    {
        for (unsigned b = 0; b < bi_count; ++b)
        {
            unsigned va = in[a];
            unsigned vb = bi[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            if (va != vb)
            {
                bool ab = checkEdgeExistence(va, vb);
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
                bool ba = checkEdgeExistence(vb, va);
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
    for (unsigned a = 0; a < bi_count; ++a)
    {
        for (unsigned b = a + 1; b < bi_count; ++b)
        {
            unsigned va = bi[a];
            unsigned vb = bi[b];
            unsigned alb = getVertexLabel(va);
            unsigned blb = getVertexLabel(vb);
            bool ab = checkEdgeExistence(va, vb);
            bool ba = checkEdgeExistence(vb, va);
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
                    unsigned tmp = alb;
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

#endif //LABEL_MOTIF_ENABLE==1

/*
 * LDF, NLF
 */
void Motif::BuildReverseIndex()
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

/*
 * PRINT
 */
/*
- each vertex: 
    label; (label print as char: labelID+'A')
    in out bi neighbors;
    in out bi nlf_struct(in egonetwork:each label freq); 
*/
void Motif::printGraphDetail()
{
    Graph::printGraphDetail(); //label, neighbors
    std::cout << "-----" << std::endl;
    std::cout << std::endl;
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        std::cout << "v" << i + 1 << ":" << std::endl;
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
        std::cout << "-----" << std::endl;
    }
}

//label, neighbors
void Motif::printGraphBasicDetail()
{
    Graph::printGraphDetail(); //label, neighbors
}

#if LABEL_MOTIF_ENABLE == 1
/*print label_motif_count_:
label print as char: labelID+'A'
code print as: topoMotifType_label_label

use label_motif_count_sz_[vid] as dime
- each vertex: label_motif_count_, with dime entries
*/
void Motif::printLabelMotifCount()
{
    char spilt_c = '\t';

    unsigned dime;
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        std::cout << "v" << i + 1 << ":" << std::endl;
        Lmtf *arr = getVertexLabelMotifCount(i, dime);
        for (unsigned j = 0; j < dime; ++j)
        {
            unsigned tmpcode = arr[j].code;
            unsigned d = tmpcode >> (2 * LABEL_BIT_WIDTH + 1);
            unsigned mask = (1 << LABEL_BIT_WIDTH) - 1; //低LABEL_BIT_WIDTH位全1
            unsigned lid1 = (tmpcode >> LABEL_BIT_WIDTH) & mask;
            unsigned lid2 = tmpcode & mask;
            std::cout
                << d << "_" << (char(lid1 + 'A')) << "_" << char(lid2 + 'A') << spilt_c;
        }
        std::cout << std::endl;
        for (unsigned j = 0; j < dime; ++j)
        {
            std::cout << arr[j].cnt << spilt_c;
        }
        std::cout << std::endl;
    }
}

#if LABEL_MOTIF_LIMIT == 1
void Motif::printLabelMotifMap()
{
    std::map<unsigned, unsigned> *pmap;
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        std::cout << "v" << i + 1 << ":\n";
        pmap = label_motif_map_ + i;
        unsigned mask = (1 << LABEL_BIT_WIDTH) - 1;
        for (auto item : *pmap)
        {
            unsigned code = item.first;
            std::cout << (code >> LABEL_SEG_WIDTH) << "_" << char(((code >> LABEL_BIT_WIDTH) & mask) + 'A') << "_" << char((code & mask) + 'A') << " ";

            std::cout << item.second << "\n";
        }
        std::cout << "\n";
    }
}
#endif //LABEL_MOTIF_LIMIT==1

#endif //LABEL_MOTIF_ENABLE==1

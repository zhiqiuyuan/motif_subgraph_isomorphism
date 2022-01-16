#ifndef _MOTIF_H
#define _MOTIF_H

#include "../util/types.h"
#include "../config/config.h"
#include "Graph.h"

class Motif : public Graph
{
protected:
#if TOPO_MOTIF_ENABLE == 1
    unsigned **motif_count_; //|V|*MOTIF_COUNT_DEMENSION
    //id为vid的顶点的MOTIF_COUNT_DEMENSION维motif_count_数组首地址为motif_count_[vid]
#endif //TOPO_MOTIF_ENABLE==1

#if LABEL_MOTIF_ENABLE == 1
    //三边特征
    Lmtf **label_motif_count_;
    //每个vertex一个Lmtf数组
    unsigned *label_motif_count_sz_; //label_motif_count_数组每行有多少entries
#if LABEL_MOTIF_LIMIT == 1
    //每个顶点一个map:code->cnt
    std::map<unsigned, unsigned> *label_motif_map_;
#endif //LABEL_MOTIF_LIMIT==1
#endif //LABEL_MOTIF_ENABLE == 1

/*
 * TOPO_MOTIF_ENABLE
 */
#if TOPO_MOTIF_ENABLE == 1
public:
    /* filename_prefix:xxx/youtube
     * 覆盖写
     * 离线：结果写入xxx/youtube_topo.txt
     *      如果WRITE_TO_FILE_DEBUG则结果还会写入xxx/youtube_topo_debug.txt
     * 在线：不写文件，给申请空间；
     *       数所有特征存unsigned **motif_count_;中；
     *       如果WRITE_TO_FILE_DEBUG则结果会写入xxx/youtube_topo_debug.txt，对于每种motif记录当初计数时是计算了哪两个顶点
     *      WRITE_TO_FILE_DEBUG为假则无需传入filename_prefix
     * 返回：返回是否计算成功（需要写文件时未传入文件名、文件不存在会失败）
     */
    bool generateTopoMotifCount(std::string filename_prefix = "");

protected:
    /* for vertex i
    * value in motif_cnt needs to be already specified
    * memory it points to needs to be already allocated
    */
    void directed_motif_count(unsigned i, unsigned *motif_cnt);
    //motif_detail:对于每种motif记录当初计数时是计算了哪两个顶点的
    void directed_motif_count_debug(unsigned i, unsigned *motif_cnt, std::vector<std::vector<std::pair<int, int>>> &motif_detail);
#endif // TOPO_MOTIF_ENABLE

/*
 * LABEL_MOTIF_ENABLE
 */
#if LABEL_MOTIF_ENABLE == 1
public:
    /* WARNNING:此函数仅为在线逻辑（数所有特征存Lmtf **label_motif_count_和unsigned *label_motif_count_sz_中），离线逻辑尚未实现
    * filename_prefix:xxx/youtube
    * 覆盖写
    * 离线尚未实现，预期结果写入xxx/youtube_label.txt
    * 在线：不写文件，给申请空间；
    *       数所有特征存Lmtf **label_motif_count_和unsigned *label_motif_count_sz_中；
    *       如果WRITE_TO_FILE_DEBUG则结果会写入xxx/youtube_label_codeInChar.txt和xxx/youtube_label.txt，并统计最大最小特征种类数（写入文件尾）
    *       WRITE_TO_FILE_DEBUG为假则无需传入filename_prefix
    * 返回：返回是否计算成功（需要写文件时未传入文件名、文件不存在会失败）
    */
    bool generateLabelMotifCount(std::string filename_prefix = "");

#if LABEL_MOTIF_LIMIT == 1
    /* filename_prefix:xxx/youtube
    * 覆盖写
    * 离线：数所有特征，以file format排版写入xxx/youtube_label_limit.txt；
    *       如果WRITE_TO_FILE_DEBUG则结果也会写入xxx/youtube_label_limit_codeInChar.txt
    * 在线：不写文件，给申请空间；
    *       数所有特征存std::map<unsigned, unsigned> *label_motif_map_中；
    *       如果WRITE_TO_FILE_DEBUG则结果会写入xxx/youtube_label_limit_codeInChar.txt，并统计最大最小特征种类数（写入文件尾）
    *       WRITE_TO_FILE_DEBUG为假则无需传入filename_prefix
    * 返回：返回是否计算成功（需要写文件时未传入文件名、文件不存在会失败）
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
    bool generateLabelMotifCount_limit(std::string filename_prefix = "");

#endif //LABEL_MOTIF_LIMIT==1

protected:
    /* 顶点i的所有特征都写入cnt_map_arr
    * for vertex i
    * value in cnt_map_arr needs to be already specified
    * memory it points to needs to be already allocated
    */
    void directed_label_motif_count(unsigned i, std::map<unsigned, unsigned> *cnt_map_arr);

#if LABEL_MOTIF_LIMIT == 1
    /* 顶点i的所有特征都写入one_vertex_cnt_map
    * for vertex i
    * value in one_vertex_cnt_map needs to be already specified
    * memory it points to needs to be already allocated
    */
    void directed_label_motif_count_limit(unsigned i, std::map<unsigned, unsigned> *one_vertex_cnt_map);
    //还记录具体是哪两个顶点
    void directed_label_motif_count_limit_debug(unsigned i, std::map<unsigned, unsigned> *one_vertex_cnt_map, std::map<unsigned, std::vector<unsigned>> *code2vertexvertex);
#endif //LABEL_MOTIF_LIMIT==1

#endif //LABEL_MOTIF_ENABLE==1

public:
    Motif();
    virtual ~Motif();

/*
 * GET,ALLOC,FREE
 */
#if TOPO_MOTIF_ENABLE == 1
    unsigned *getVertexMotifCount(const VertexID id) const
    {
        return motif_count_[id];
    }
#endif //TOPO_MOTIF_ENABLE==1

#if LABEL_MOTIF_ENABLE == 1
    Lmtf *getVertexLabelMotifCount(const VertexID id) const
    {
        return label_motif_count_[id];
    }
    Lmtf *getVertexLabelMotifCount(const VertexID id, unsigned &cnt) const
    {
        cnt = label_motif_count_sz_[id];
        return label_motif_count_[id];
    }

#if LABEL_MOTIF_LIMIT == 1
    std::map<unsigned, unsigned> *getVertexLabelMotifMap(const VertexID id) const
    {
        return label_motif_map_ + id;
    }
#endif //LABEL_MOTIF_LIMIT==1
    void alloc_label_motif_count()
    {
        label_motif_count_ = new Lmtf *[vertices_count_];
        for (unsigned i = 0; i < vertices_count_; ++i)
        {
            label_motif_count_[i] = new Lmtf[1];
        }
    }
    void delete_label_motif_count_()
    {
        delete[] label_motif_count_;
    }
#endif //LABEL_MOTIF_ENABLE==1

    /*
 * PRINT
 */
#if LABEL_MOTIF_ENABLE == 1
    /*print label_motif_count_:
    label print as char: labelID+'A'
    code print as: topoMotifType_label_label

    use label_motif_count_sz_[vid] as dime
    - each vertex: label_motif_count_, with dime entries
    */
    void printLabelMotifCount();
#if LABEL_MOTIF_LIMIT == 1
    void printLabelMotifMap();
#endif //LABEL_MOTIF_LIMIT == 1
#endif //LABEL_MOTIF_ENABLE==1
};

#endif //_MOTIF_H
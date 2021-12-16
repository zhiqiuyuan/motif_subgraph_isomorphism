#ifndef _MOTIFG_H
#define _MOTIFG_H

#include "Motif.h"

class MotifG : public Motif
{
protected:
public:
    MotifG();
    virtual ~MotifG();

#if TOPO_MOTIF_ENABLE == 1
    /* filename_prefix:xxx/youtube，从xxx/youtube_topo.txt中加载到unsigned **motif_count_; 
    file format: 
    VertexID MOTIF_COUNT_DEMENSIONcount
    */
    void loadTopoMotifCountFromFile(std::string filename_prefix);
#endif //#if TOPO_MOTIF_ENABLE == 1

#if LABEL_MOTIF_ENABLE == 1
    /* WARNNING:此函数注释掉的实现的逻辑为分配空间并从youtube_directed_label_kmethod_kInFileName中加载motif结构，此函数还没有实现
    * filename_prefix: xxx/youtube,xxx/youtube_label.txt中加载到Lmtf **label_motif_count_和label_motif_count_sz_
    */
    void loadLabelMotifCountFromFile(std::string filename_prefix);

#if LABEL_MOTIF_LIMIT == 1
    /* 从xxx/youtube_label_limit.txt中加载到std::map<unsigned, unsigned> *label_motif_map_
    * filename_prefix:xxx/youtube
    * 在此函数中给label_motif_map_分配空间
    */
    /*
    file format:
    vid code_cnt
    code cnt code cnt……
    */
    void loadLabelMotifCountFromFile_limit(std::string filename_prefix);
#endif //#if LABEL_MOTIF_LIMIT == 1
#endif //#if LABEL_MOTIF_ENABLE == 1
};

#endif //_MOTIFG_H
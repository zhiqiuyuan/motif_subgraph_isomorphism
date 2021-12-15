#ifndef SUBGRAPHMATCHING_CONFIG_H
#define SUBGRAPHMATCHING_CONFIG_H

#include <iostream>
#include <vector>

/**
 * Setting the value as 1 is to (1) enable the neighbor label frequency filter (i.e., NLF filter); and (2) enable
 * to check the existence of an edge with the label information. The cost is to (1) build an unordered_map for each
 * vertex to store the frequency of the labels of its neighbor; and (2) build the label neighbor offset.
 * If the memory can hold the extra memory cost, then enable this feature to boost the performance. Otherwise, disable
 * it by setting this value as 0.
 */
#define OPTIMIZED_LABELED_GRAPH 1

#define DIRECTED_GRAPH 1 //1:有向图 0:无向图（为1（或0）会屏蔽无向图（或有向图）相关函数和数据结构的定义）
#define TOPO_MOTIF_ENABLE 0 
#define LABEL_MOTIF_ENABLE 1 //1:使用标签motif（为0 不会 屏蔽LMTF相关函数和数据结构的定义，只是不使用）

/*
q批量时q文件名：query_sparse_qVertexScale_j.graph
*/
//std::vector<unsigned> DEFAULT_QUERY_VERTEXSCALE_SET = {8, 16, 24, 36}; //q批量时若用户选择default则使用此作为q_vertexScale_set
#define DEFAULT_JB 1   //q批量时若用户选择default则使用此作为jb
#define DEFAULT_JE 200 //q批量时若用户选择default则使用此作为je

/*
directed topo
*/
#define MOTIF_COUNT_DEMENSION 15 //24 //有向图不考虑标签motif种类数

/*
directed label motif
*/
//label motif种类编码
#define LABEL_BIT_WIDTH 14                  //13 //label motif encoding中：单个labelID段的bit位数
#define LABEL_SEG_WIDTH LABEL_BIT_WIDTH * 2 //label段bit位数

//G选k种lmotif写入文件的选k种方法
#define DEFAULT_KMETHOD_NUM 1 //size of DEFAULT_KMETHOD_NUM
//std::string DEFAULT_KMETHOD_SET[DEFAULT_KMETHOD_NUM] = {"top", "down", "mid", "rand"}; //default
//std::string* KMETHOD_SET;                                          //实际输入之后用这个
//ui KMETHOD_NUM;                                                                        //实际输入之后用这个 //KMETHOD_NUM一定<=DEFAULT_KMETHOD_NUM（因为KMETHOD_SET必须是DEFAULT_KMETHOD_SET的子集）

#define KMAX 200 //G图最多每个点存KMAX种lmtf

#define ONLINE_STAGE 1 
/*
1:在线查询阶段，
	generateMotifCount和generateMotifCount_label会写入文件，并且全部motif_cnt会申请空间保留
	main函数：在线查询逻辑
0:离线计算存储阶段，
	generateMotifCount和generateMotifCount_label会写入文件，并且写入文件的部分不继续保留而是释放
	main函数：离线计算逻辑
*/

#define ON_LINUX 1

//#define DEBUG
//#define ONLINE_DEBUG
#define WRITE_TO_FILE_DEBUG 0 
#define INPUT_PARA_FROM_SRCCODE 0
#define COLLECT_DATA_FEATURE 0

#define LABEL_MOTIF_LIMIT 1 //数据图也存所有的标签特征，看标签motif过滤效果的上限

#endif //SUBGRAPHMATCHING_CONFIG_H

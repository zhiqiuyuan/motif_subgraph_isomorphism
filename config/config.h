#ifndef _CONFIG_H
#define _CONFIG_H

/*
ENABLE
1:使能对应motif 
0:不使能，不会参与编译
*/
#define TOPO_MOTIF_ENABLE 1
#define LABEL_MOTIF_ENABLE 1 //标签motif
#define LABEL_MOTIF_LIMIT 1	 //数据图也存所有的标签特征，看标签motif过滤效果的上限

#define COLLECT_G_FEATURE 1 //统计数据图特征
#define COLLECT_Q_FEATURE 1 //统计查询图特征

#define ONLINE_STAGE 1
/* 初衷：数据图在离线阶段有些结构没有申请空间+同名的generatemotif函数在离线和在线阶段有些逻辑不一样
1:在线查询阶段，
	generateMotifCount和generateMotifCount_label会写入文件，并且全部motif_cnt会申请空间保留
	main函数：在线查询逻辑
0:离线计算存储阶段，
	generateMotifCount和generateMotifCount_label会写入文件，并且写入文件的部分不继续保留而是释放
	main函数：离线计算逻辑
*/

/*
PARAMETERS
*/
//有向图不考虑标签motif种类数
#define MOTIF_COUNT_DEMENSION 15 //24
//label motif种类编码
#define LABEL_BIT_WIDTH 14					//13 //label motif encoding中：单个labelID段的bit位数
#define LABEL_SEG_WIDTH LABEL_BIT_WIDTH * 2 //label段bit位数

//q批量时q文件名：query_sparse_qVertexScale_j.graph
#define DEFAULT_JB 1   //q批量时若用户选择default则使用此作为jb
#define DEFAULT_JE 200 //q批量时若用户选择default则使用此作为je

/*
DEBUG
*/
#define WRITE_TO_FILE_DEBUG 1	//计算motif结构时会以好检查的形式写入_debug文件中
#define RUNNING_COMMENT 1		//运行时输出表示运行程度的信息（如计算Motif结构时计算好了多少顶点）
#define BASIC_RUNNING_COMMENT 1 //输出基础的表示运行程度的信息（如现在在加载数据图；输出信息是RUNNING_COMMENT==1时的子集）
#define STEP_DEBUG 0			//细粒度一步一步输出状态

#endif //_CONFIG_H

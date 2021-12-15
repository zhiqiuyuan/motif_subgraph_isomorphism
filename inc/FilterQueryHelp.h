#ifndef FILTERQUERYHELP_H
#define FILTERQUERYHELP_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "types.h"
#include "Graph.h"
#include "FilterVertices.h"

/*======================
返回candidates_count的前vertex_num个元素的平均值
*/
double get_average(ui *candidates_count, ui vertex_num);
double get_average(double *candidates_count, ui vertex_num);

/*======================
acculmulating moving average
*/
/*
cnt: num of values included in old_ave
初始：cnt设置为0，new_val传第 一个元素(第一个数组)，old_ave 随意给（比如给0）
*/
void moving_average(double &old_ave, double new_val, ui &cnt);         //return new_ave
void moving_average(double *old_ave, double *new_val, ui &cnt, ui sz); //modify old_ave
void moving_average(kMethodCandiScale *old_ave, kMethodCandiScale *new_val, ui &cnt, ui sz);

/*======================
motif考虑标签有向图
*/
#if DIRECTED_GRAPH == 1 && LABEL_MOTIF_ENABLE == 1 && ONLINE_STAGE == 1

/*
非批量
计算候选解规模
data_graph已经加载；query_graph已经加载，已经计算好motif_struct
filename_prefix:"xxx/youtube"，会从xxx/youtube_directed_label_top_k.txt中加载G的motif_struct
会给lmtf_candiScale_arr分配空间
lmtf_candiScale_arr: each (k,kmethod) an entry
*/
void dLmtf_filterCandi_vary_k_method(std::string filename_prefix, kMethodCandiScale *&lmtf_candiScale_arr, ui kb, ui ke, ui kInFileName, std::string *method_set, ui KMETHOD_NUM, Graph *data_graph, Graph *query_graph);

/*
批量
计算候选解规模并打印diff
data_graph已经加载
[int jb, int je]:指明q_set：[jb,je]:q图的顶点规模
filename_prefix:"xxx/youtube"，会从xxx/youtube_directed_label_top_k.txt中加载G的motif_struct
qfilename_prefix:"xxx/SPARSE"，会从xxx/query_sparse_vertexScale_j.graph加载q，会将结果写入xxx/query_sparse_vertexScale_j_directed（或undirected）.txt中
会给lmtf_ave_candiScale_arr分配空间
lmtf_ave_candiScale_arr：each (k,kmethod) an entry
*/
//800*800次加载G的motif_cnt，800次加载q和计算q的motif_cnt
void bulkq_dLmtf_filterCandi_vary_k_method_v1(std::string filename_prefix, std::string qfilename_prefix, kMethodCandiScale *&lmtf_ave_candiScale_arr, ui kb, ui ke, ui kInFileName, std::string *method_set, ui KMETHOD_NUM, Graph *data_graph, ui qVScale, int jb, int je);
//800次加载G的motif_cnt，800*800次加载q和计算q的motif_cnt
//把k-kmethod循环放到q循环外
void bulkq_dLmtf_filterCandi_vary_k_method(std::string filename_prefix, std::string qfilename_prefix, kMethodCandiScale *&lmtf_ave_candiScale_arr, ui kb, ui ke, ui kInFileName, std::string *method_set, ui KMETHOD_NUM, Graph *data_graph, ui qVScale, int jb, int je, bool one_step);
void bulkq_dLmtf_filterCandi_vary_k_method_collect_data_feature(std::string filename_prefix, std::string qfilename_prefix, kMethodCandiScale *&lmtf_ave_candiScale_arr, ui kb, ui ke, ui kInFileName, std::string *method_set, ui KMETHOD_NUM, Graph *data_graph, ui qVScale, int jb, int je, ui *feature);

#if LABEL_MOTIF_LIMIT == 1
void bulkq_lmtf_limit_filter(std::string qfilename_prefix, Graph *data_graph, ui qVScale, int jb, int je, double &mtf_candiScale_ave);
#endif //LABEL_MOTIF_LIMIT
/*
打印diff（会先按候选解规模升序排序）
6列一面
*/
void print_scale_and_diff(double nlf_candiScale, std::string other_comment, kMethodCandiScale *lmtf_candiScale_arr, std::string lmtf_comment, std::string *method_set, ui sz_of_lmtf_ave_candiScale_arr);
void print_scale_and_diff_inner(double nlf_candiScale, std::string other_comment, kMethodCandiScale *lmtf_candiScale_arr, std::string lmtf_comment, std::string *method_set, ui sz);

/*
(k,kmethod)候选解规模写入文件
result_fname_prefix:/media/data/hnu2022/yuanzhiqiu/filter_candiScale/
dg_name:youtube
sparse_str:SPARSE DENSE
*/
/*
（所有数据库的结果写到一个文件/media/data/hnu2022/yuanzhiqiu/filter_candiScale/lmtf_CandiScale.txt里面）
[candiScale ascending -> k ascending]
youtube
qVScale1 S/D
kmethod1 k1 candiScale1 
kmethod2 k2 candiScale2 
...
youtube
qVScale2 S/D
...
hprd
...
*/
void write_lmtf_CandiScale(kMethodCandiScale *lmtf_ave_candiScale_arr, std::string *method_set, ui sz, ui q_vertexScale, std::string sparse_str, std::string result_fname_prefix, std::string dg_name, std::string filterTypeName, double duration);
#endif //#if DIRECTED_GRAPH == 1 && LABEL_MOTIF_ENABLE == 1 && && ONLINE_STAGE==1

/*======================
NLF批量
*/
/*
data_graph已经加载
qfilename_prefix:"xxx/SPRASE"，会从xxx/query_sparse_vertexScale_j.graph加载q
*/
void bulkq_nlf_filterCandi(std::string qfilename_prefix, double &nlf_ave_candiScale, Graph *data_graph, ui qVScale, int jb, int je, bool one_step);
void bulkq_ldf_filterCandi(std::string qfilename_prefix, double &nlf_ave_candiScale, Graph *data_graph, ui qVScale, int jb, int je);

/*======================
motif其他(不考虑标签有向图，考虑标签无向图，不考虑标签无向图)
*/
#if (DIRECTED_GRAPH == 1 || LABEL_MOTIF_ENABLE == 1) && ONLINE_STAGE == 1
/*
非批量
data_graph已经加载，已经加载motif结构；query_graphy已经加载
qfilename:"xxx/query_sparse_vertexScale_j.graph"，会从qfilename加载q，会将结果写入xxx/query_sparse_vertexScale_j_directed或undirected.txt中
*/
void mtf_nlf_filter(std::string qfilename, Graph *data_graph, Graph *query_graph, bool label, double &mtf_candiScale, double &nlf_candiScale);
/*
批量
data_graph已经加载，已经加载motif结构
qfilename_prefix:"xxx/SPARSE"，会从xxx/query_sparse_vertexScale_j.graph加载q，会将结果写入xxx/query_sparse_vertexScale_j_directed（或undirected）.txt中
*/
void bulkq_mtf_nlf_filter(std::string qfilename_prefix, Graph *data_graph, ui qVScale, int jb, int je, bool label, double &mtf_candiScale_ave, double &nlf_candiScale_ave);
void bulkq_mtf_filter(std::string qfilename_prefix, Graph *data_graph, ui qVScale, int jb, int je, double &mtf_candiScale_ave, bool do_one_step, bool use_nlf);
void print_scale_and_diff(double mtf_candiScale, std::string mtf_comment, double nlf_candiScale, std::string nlf_comment);

//一面6列
void print_scale_and_diff(std::vector<ui> q_vertexScale_set, double *mtf_ave_candiScale_arr, std::string mtf_comment, double *nlf_ave_candiScale_arr, std::string nlf_comment);
void print_scale_and_diff_inner(std::vector<ui> q_vertexScale_set, double *mtf_ave_candiScale_arr, std::string mtf_comment, double *nlf_ave_candiScale_arr, std::string nlf_comment);

#endif //#if (DIRECTED_GRAPH == 1 || LABEL_MOTIF_ENABLE == 1) && ONLINE_STAGE==1

/*========================
input (user interface)
*/
bool input_kmethod_set(std::string *&method_set, ui &KMETHOD_NUM, std::string *DEFAULT_KMETHOD_SET);
void input_kb_ke(ui &kb, ui &ke, ui min_lmtf_kind, ui max_lmtf_kind);

/*========================
print
*/
//print candidates [qvertex*candi_for_each]
//sz:size of candidates_num
//candidates_num: candidates_num[i] is size of candidates[i]
void print_candidates(ui **candidates, ui *candidates_num, ui sz);

/*========================
文件
*/
//移动光标至文件最后一行的开头（当前字符是上一行的最后一个字符，ios_base:cur指向这最后一行的第一个字符）
void goto_first_of_last_line(std::ifstream &fin);
void write_time(ui q_vertexScale, std::string sparse_str, std::string result_fname_prefix, std::string dg_name, std::string filterTypeName, double duration);

/*========================
linux相关
*/
//获取系统一个进程可以打开的最大文件数
int getSysMaxOpenFilesNum();

#endif //FILTERQUERYHELP_H

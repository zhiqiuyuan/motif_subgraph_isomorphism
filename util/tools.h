#include "types.h"

/*
* MATH
*/
//return [0,possible_max)
long long Rand(long long possible_max);

//concate as left+right: pick element from left+right at random
unsigned getRandFrom2Array(const unsigned *left, const unsigned *right, unsigned left_sz, unsigned right_sz);

/*
返回candidates_count的前vertex_num个元素的平均值
*/
double get_average(unsigned *candidates_count, unsigned vertex_num);
double get_average(double *candidates_count, unsigned vertex_num);

/*
acculmulating moving average
modify old_ave
cnt: num of values included in old_ave
初始：cnt设置为0，new_val传第 一个元素(第一个数组)，old_ave 随意给（比如给0）
*/
void moving_average(double &old_ave, double new_val, unsigned &cnt);
void moving_average(double *old_ave, double *new_val, unsigned &cnt, unsigned sz);

/*
* PRINT
*/
/* print candidates [qvertex*candi_for_each]
 * sz:size of candidates_num
 * candidates_num: candidates_num[i] is size of candidates[i]
 */
void printCandidates(unsigned **candidates, unsigned *candidates_num, unsigned sz);
/* 输出markdown式表格到标准输出
* head:表头
*/
void printMdTables(std::vector<std::string> head, unsigned **data, int rowsz, std::string textBeforeTable);
/*
* FILE
*/
//移动光标至文件最后一行的开头（当前字符是上一行的最后一个字符，ios_base:cur指向这最后一行的第一个字符）
void goto_first_of_last_line(std::ifstream &fin);

/*
* LINUX
*/
//获取系统一个进程可以打开的最大文件数
int getSysMaxOpenFilesNum();
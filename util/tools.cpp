#include "tools.h"

/* MATH
*/
//return [0,possible_max)
long long Rand(long long possible_max)
{
    //segment division
    double min_interval_width = possible_max / (double)RAND_MAX;
    long long re = rand();
    while (min_interval_width >= RAND_MAX)
    {
        re = re * RAND_MAX + rand();
        min_interval_width /= RAND_MAX;
    }
    return re % possible_max;
}

//concate as left+right: pick element from left+right at random
unsigned getRandFrom2Array(const unsigned *left, const unsigned *right, unsigned left_sz, unsigned right_sz)
{
    unsigned idx = Rand(left_sz + right_sz);
    if (idx < left_sz)
    {
        return left[idx];
    }
    return right[idx - left_sz];
}

/*
返回candidates_count的前vertex_num个元素的平均值
*/
double get_average(unsigned *candidates_count, unsigned vertex_num)
{
    if (vertex_num < 1)
    {
        return 0;
    }
    double ave = candidates_count[0];
    double cnt = 1;
    for (unsigned i = 1; i < vertex_num; ++i)
    {
        ++cnt;
        double tmp1 = ave * ((double(cnt - 1)) / cnt);
        double tmp2 = candidates_count[i] / cnt;
        ave = tmp1 + tmp2;
    }
    return ave;
}
double get_average(double *candidates_count, unsigned vertex_num)
{
    if (vertex_num < 1)
    {
        return 0;
    }
    double ave = candidates_count[0];
    double cnt = 1;
    for (unsigned i = 1; i < vertex_num; ++i)
    {
        ++cnt;
        double tmp1 = ave * ((double(cnt - 1)) / cnt);
        double tmp2 = candidates_count[i] / cnt;
        ave = tmp1 + tmp2;
    }
    return ave;
}

/*
acculmulating moving average
modify old_ave
cnt: num of values included in old_ave
初始：cnt设置为0，new_val传第 一个元素(第一个数组)，old_ave 随意给（比如给0）
*/
void moving_average(double &old_ave, double new_val, unsigned &cnt)
{
    ++cnt;
    double tmp1 = old_ave * ((double(cnt - 1)) / cnt);
    double tmp2 = new_val / cnt;
    old_ave = tmp1 + tmp2;
}
void moving_average(double *old_ave, double *new_val, unsigned &cnt, unsigned sz)
{
    ++cnt;
    for (unsigned i = 0; i < sz; ++i)
    {
        double tmp1 = old_ave[i] * ((double(cnt - 1)) / cnt);
        double tmp2 = new_val[i] / cnt;
        old_ave[i] = tmp1 + tmp2;
    }
}

/* PRINT
*/
/* print candidates [qvertex*candi_for_each]
 * sz:size of candidates_num
 * candidates_num: candidates_num[i] is size of candidates[i]
 */
void printCandidates(unsigned **candidates, unsigned *candidates_num, unsigned sz)
{
    unsigned num;
    char split_c = '\t';
    for (unsigned i = 0; i < sz; ++i)
    {
        num = candidates_num[i];
        std::cout << "u" << i + 1 << ":" << std::endl;
        for (unsigned j = 0; j < num; ++j)
        {
            std::cout << "v" << candidates[i][j] + 1 << split_c;
        }
        std::cout << std::endl;
    }
}

void printMdTables(std::vector<std::string> head, unsigned **data, int rowsz, std::string textBeforeTable)
{
    std::cout << textBeforeTable << std::endl;

    std::cout << "|";
    for (auto h : head)
    {
        std::cout << " " << h << " |";
    }
    std::cout << std::endl;

    int colsz = head.size();
    std::cout << "|";
    for (int i = 0; i < colsz; ++i)
    {
        std::cout << " -- |";
    }
    std::cout << std::endl;

    for (int i = 0; i < rowsz; ++i)
    {
        std::cout << "|";
        for (int j = 0; j < colsz; ++j)
        {
            std::cout << " " << data[i][j] << " |";
        }
        std::cout << std::endl;
    }
}

/*
* FILE
*/
//移动光标至文件最后一行的开头（当前字符是上一行的最后一个字符，ios_base:cur指向这最后一行的第一个字符）
void goto_first_of_last_line(std::ifstream &fin)
{
    fin.seekg(-1, std::ios_base::end);
    /*
    123456 [last line]
         ^(ios_base::cur points here)
    */
    //move to the first char in last line
    char c;
    while (1)
    {
        c = fin.get(); //move ios_base::cur one char further(therefore the seekg(-2) below)
        /*eg:in first loop
        123456 [last line]
              ^(ios_base::cur points here)
        */
        if (int(fin.tellg()) <= 1)
        {
            //only one line
            /*now
            123456 [the only line in fin]
             ^(ios_base::cur points here)
            seekg(-2) will be illegal
            */
            fin.seekg(std::ios_base::beg);
            break;
        }
        if (c == '\n')
        {
            break; //cur is ok (pointing to the char after this newline)
        }
        fin.seekg(-2, std::ios_base::cur);
    }
}

/*
* LINUX
*/
//获取系统一个进程可以打开的最大文件数
int getSysMaxOpenFilesNum()
{
    char line[30];
    FILE *fp;
    std::string cmd = "ulimit -n";
    const char *sysCommand = cmd.data();
    if ((fp = popen(sysCommand, "r")) == NULL)
    {
        std::cout << "get 'ulimit -n' failed" << std::endl;
        return -1;
    }
    std::string nofile_str;
    int nofile;
    while (fgets(line, sizeof(line) - 1, fp) != NULL)
    {
        nofile_str = std::string(line);
        nofile = std::stoi(nofile_str);
    }
    pclose(fp);
    return nofile;
}

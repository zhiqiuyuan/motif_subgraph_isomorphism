#ifndef _BULKQUERIES_H
#define _BULKQUERIES_H

#include "../MotifGraph/MotifG.h"
#include "../MotifGraph/MotifQ.h"
#include "../TVEFile/TVEFileQ.h"
#include "../Matching/Filter.h"
#include "../util/tools.h"

class BulkQueries
{
protected:
    //qfilename_pre：xxx/query_isSparse_qVScale_，会从xxx/query_isSparse_qVScale_j.graph加载q
    std::string qfilename_pre;
    double ave_candiScale;
    MotifG *data_graph;
    int jb, je; //j[jb,je]

    unsigned qVScale;
    bool isSparse;

public:
    //qfilename_prefix: xxx/SPRASE
    BulkQueries(std::string qfilename_prefix, MotifG *data_graph0, int jb0, int je0, unsigned qVScale0 = 0);
    //MotifG *data_graph需要用户释放
    virtual ~BulkQueries();

    void setQVScale(unsigned qVScale0)
    {
        qVScale = qVScale0;
    }
    void setJb(int jb0)
    {
        jb = jb0;
    }
    void setJe(int je0)
    {
        je = je0;
    }

    /* FILTER   
    */
    /* bulk queries
    * 要求data_graph已经加载基本结构和离线结构
    * 由于查询图是通过util/GenerateTVEFileQ中的函数生成的，生成格式为TVE格式，所以这里写死申请的查询图为TVEFileQ类型
    * filterMethod目前支持: ldf,tmtf,nlf,lmtf_limit
    * 
    * 使用方法：调用后取ave_candiScale即可
    */
    void bulkq_filter_forAveCandiScale(std::string filterMethod);

    double get_ave_candiScale()
    {
        return ave_candiScale;
    }
};

#endif //_BULKQUERIES_H

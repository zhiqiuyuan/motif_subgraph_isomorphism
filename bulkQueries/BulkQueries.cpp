#include "BulkQueries.h"

//qfilename_prefix: xxx/SPRASE
BulkQueries::BulkQueries(std::string qfilename_prefix, MotifG *data_graph0, int jb0, int je0, unsigned qVScale0) : data_graph(data_graph0), qVScale(qVScale0), jb(jb0), je(je0), ave_candiScale(0)
{
        //qfilename_pre：xxx/query_isSparse_qVScale_
        unsigned idx = qfilename_prefix.find_last_of('/');
        std::string filename = qfilename_prefix.substr(idx + 1);        //SPARSE或DENSE
        qfilename_prefix = qfilename_prefix.substr(0, idx) + "/query_"; //xxx/query_
        if (filename == "SPARSE")
        {
                filename = qfilename_prefix + "sparse_"; //xxx/query_sparse_
                isSparse = 1;
        }
        else if (filename == "DENSE")
        {
                filename = qfilename_prefix + "dense_"; //xxx/query_dense_
                isSparse = 0;
        }
        filename += std::to_string(qVScale) + "_"; //xxx/query_sparse_qVScale_
        qfilename_pre = filename;
}

BulkQueries::~BulkQueries()
{
}

bool BulkQueries::qset_is_weakConnected(std::string startVertexSelectMethod)
{
        unsigned start = 0;

        std::string filename;
        for (int j = jb; j <= je; ++j)
        {
                filename = qfilename_pre + std::to_string(j) + ".graph";
                TVEFileQ *query_graph = new TVEFileQ();
                query_graph->loadGraphFromFile(filename);
                /*
                if (startVertexSelectMethod == "cfl")
                {
                }
                else if (startVertexSelectMethod == "dpiso")
                {
                }
                */
                if (GraphFeatures::isWeakConnectedFromRoot(query_graph, start) == 0)
                {
                        std::cout << filename << std::endl;
                        return 0;
                }
                delete query_graph;
        }
        return 1;
}

void BulkQueries::bulkq_filter_forAveCandiScale(std::string filterMethod)
{
        ave_candiScale = 0;
        double new_candiScale;
        unsigned cnt = 0;
        std::string filename;
        for (int j = jb; j <= je; ++j)
        {
                filename = qfilename_pre + std::to_string(j) + ".graph";
#if STEP_DEBUG == 1
                std::cout << filename << std::endl;
                std::cout << "do " << filterMethod << " for qj:" << j << std::endl;
#endif //#if STEP_DEBUG==1
                TVEFileQ *query_graph = new TVEFileQ();
                query_graph->loadGraphFromFile(filename);

                if (filterMethod == "ldf")
                {
                        new_candiScale = Filter::LDFFilter_AveCandiScale(data_graph, query_graph);
                }
                else if (filterMethod == "tmtf")
                {
#if TOPO_MOTIF_ENABLE == 0
                        std::cout << __FILE__ << " :" << __LINE__;
                        std::cout << "\n\tTOPO_MOTIF_ENABLE==0, tmtf not supported, return. \n\tedit config/config.h\n";
                        return;
#endif //#if TOPO_MOTIF_ENABLE==0
#if WRITE_TO_FILE_DEBUG == 1
                        new_candiScale = Filter::TopoMotifFilter_AveCandiScale(data_graph, query_graph, filename);
#else
                        new_candiScale = Filter::TopoMotifFilter_AveCandiScale(data_graph, query_graph);
#endif //#if WRITE_TO_FILE_DEBUG==1
                }
                else if (filterMethod == "nlf")
                {
                        new_candiScale = Filter::NLFFilter_AveCandiScale(data_graph, query_graph);
                }
                else if (filterMethod == "lmtf_limit")
                {
#if LABEL_MOTIF_ENABLE == 0 || LABEL_MOTIF_LIMIT == 0
                        std::cout << __FILE__ << " :" << __LINE__;
                        std::cout << "\n\tLABEL_MOTIF_ENABLE == 0 || LABEL_MOTIF_LIMIT == 0, lmtf_limit not supported, return. \n\tedit config/config.h\n";
                        return;
#endif //#if LABEL_MOTIF_ENABLE == 0 || LABEL_MOTIF_LIMIT==0
#if WRITE_TO_FILE_DEBUG == 1
                        new_candiScale = Filter::LabelMotifFilter_limit_AveCandiScale(data_graph, query_graph, filename);
#else
                        new_candiScale = Filter::LabelMotifFilter_limit_AveCandiScale(data_graph, query_graph);
#endif //#if WRITE_TO_FILE_DEBUG==1
                }
                else if (filterMethod == "gql")
                {
                        new_candiScale = Filter::GQLFilter_AveCandiScale(data_graph, query_graph);
                }
                else if (filterMethod == "cfl")
                {
                        new_candiScale = Filter::CFLFilter_AveCandiScale(data_graph, query_graph);
                }
                else if (filterMethod == "dpiso")
                {
                        new_candiScale = Filter::DPisoFilter_AveCandiScale(data_graph, query_graph);
                }
                else if (filterMethod.find("_lmtf_limit") != std::string::npos)
                {
#if LABEL_MOTIF_ENABLE == 0 || LABEL_MOTIF_LIMIT == 0
                        std::cout << __FILE__ << " :" << __LINE__;
                        std::cout << "\n\tLABEL_MOTIF_ENABLE == 0 || LABEL_MOTIF_LIMIT == 0, lmtf_limit not supported, return. \n\tedit config/config.h\n";
                        return;
#endif //#if LABEL_MOTIF_ENABLE == 0 || LABEL_MOTIF_LIMIT==0
                        std::string firstStageFilterKind;
                        unsigned idx = filterMethod.find('_');
                        firstStageFilterKind = filterMethod.substr(0, idx);
#if WRITE_TO_FILE_DEBUG == 1
                        new_candiScale = Filter::LabelMotifFilter_limit_AveCandiScale(data_graph, query_graph, firstStageFilterKind, filename);
#else
                        new_candiScale = Filter::LabelMotifFilter_limit_AveCandiScale(data_graph, query_graph, firstStageFilterKind, "");
#endif //#if WRITE_TO_FILE_DEBUG==1
                }

                if (new_candiScale < 0)
                {
                        std::cout << __FILE__ << " :" << __LINE__;
                        std::cout << "\n\tin bulkq_filter, method" << filterMethod << ": negative new_candiScale, not included in moving_average, return\n";
                        return;
                }

#if STEP_DEBUG == 1
                std::cout << "this ave:" << new_candiScale << std::endl;
#endif //#if STEP_DEBUG==1
                moving_average(ave_candiScale, new_candiScale, cnt);
#if STEP_DEBUG == 1
                std::cout << "after moving, the new ave:" << ave_candiScale << std::endl;
#endif //#if STEP_DEBUG==1

                delete query_graph;
        }
}

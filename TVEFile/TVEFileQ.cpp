#include "TVEFileQ.h"

TVEFileQ::TVEFileQ()
{
}

TVEFileQ::~TVEFileQ()
{
}

void TVEFileQ::loadGraphFromFile(const std::string &file_path)
{
    std::ifstream infile(file_path);

    if (!infile.is_open())
    {
        std::cout << "Can not open the graph file " << file_path << " ." << std::endl;
        exit(-1);
    }

    char type;
    infile >> type >> vertices_count_ >> edges_count_;
    offsets_ = new unsigned[vertices_count_ + 1];
    offsets_[0] = 0;

    labels_ = new LabelID[vertices_count_];
    labels_count_ = 0;

    in_neighbors_nums_ = new unsigned[vertices_count_]();
    in_neighbors_ = new LabelID[edges_count_ * 2];
    out_neighbors_nums_ = new unsigned[vertices_count_]();
    out_neighbors_ = new LabelID[edges_count_ * 2];
    bi_neighbors_nums_ = new unsigned[vertices_count_]();
    bi_neighbors_ = new LabelID[edges_count_ * 2];

    LabelID max_label_id = 0;

    while (infile >> type)
    {
        if (type == 'v')
        { // Read vertex.
            VertexID id;
            LabelID label;
            unsigned degree; //in+out degree
            infile >> id >> label >> degree;

            labels_[id] = label;
            offsets_[id + 1] = offsets_[id] + degree;
            if (degree > max_degree_)
            {
                max_degree_ = degree;
            }

            if (labels_frequency_.find(label) == labels_frequency_.end())
            {
                labels_frequency_[label] = 0;
                if (label > max_label_id)
                    max_label_id = label;
            }

            labels_frequency_[label] += 1;
        }
        else if (type == 'e')
        { // Read edge.
            VertexID begin;
            VertexID end;
            infile >> begin >> end;

            //begin
            unsigned offset = offsets_[begin] + out_neighbors_nums_[begin];
            out_neighbors_[offset] = end;
            out_neighbors_nums_[begin]++;

            //end
            offset = offsets_[end] + in_neighbors_nums_[end];
            in_neighbors_[offset] = begin;
            in_neighbors_nums_[end]++;
        }
    }

    infile.close();
    labels_count_ = (unsigned)labels_frequency_.size() > (max_label_id + 1) ? (unsigned)labels_frequency_.size() : max_label_id + 1;

    for (auto element : labels_frequency_)
    {
        if (element.second > max_label_frequency_)
        {
            max_label_frequency_ = element.second;
        }
    }

    //sort in out according vertex id (to support bi search)
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        unsigned indegree = in_neighbors_nums_[i];
        if (indegree > max_single_degree_)
        {
            max_single_degree_ = indegree;
        }
        unsigned outdegree = out_neighbors_nums_[i];
        if (outdegree > max_single_degree_)
        {
            max_single_degree_ = outdegree;
        }

        std::sort(in_neighbors_ + offsets_[i], in_neighbors_ + offsets_[i] + indegree);
        std::sort(out_neighbors_ + offsets_[i], out_neighbors_ + offsets_[i] + outdegree);
    }
    //归并get bi
    for (unsigned i = 0; i < vertices_count_; ++i)
    {
        unsigned off = offsets_[i];
        unsigned ii = off, oi = off;
        unsigned ie = ii + in_neighbors_nums_[i];
        unsigned oe = oi + out_neighbors_nums_[i];
        unsigned inid, outid;
        while (ii < ie && oi < oe)
        {
            inid = in_neighbors_[ii];
            outid = out_neighbors_[oi];
            if (inid < outid)
            {
                ++ii;
            }
            else if (inid > outid)
            {
                ++oi;
            }
            else
            {
                bi_neighbors_[off + bi_neighbors_nums_[i]] = inid;
                bi_neighbors_nums_[i]++;
                ++ii;
                ++oi;
            }
        }
    }

    BuildReverseIndex(); //label -> all vids
    //BuildNLF();
}

#ifndef _TVEFILEQ_H
#define _TVEFILEQ_H

#include "../MotifGraph/MotifQ.h"

class TVEFileQ : public MotifQ
{
private:
public:
    TVEFileQ();
    ~TVEFileQ();
    void loadGraphFromFile(const std::string &file_path);
};

#endif //_TVEFILEQ_H

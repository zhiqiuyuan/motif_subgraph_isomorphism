#ifndef _TVEFILEG_H
#define _TVEFILEG_H

#include "../MotifGraph/MotifG.h"

class TVEFileG : public MotifG
{
private:
public:
    TVEFileG();
    ~TVEFileG();
    void loadGraphFromFile(const std::string &file_path);
};

#endif //_TVEFILEG_H

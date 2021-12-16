#ifndef _TYPES_H
#define _TYPES_H

#include <cstdint>
#include <stdlib.h>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <string.h>
#include <time.h>
#include <memory.h>

#include <unistd.h>

typedef unsigned VertexID;
typedef unsigned LabelID;

struct Lmtf
{
    unsigned code;
    unsigned cnt;
};
bool code_ascending(Lmtf l, Lmtf r)
{
    return l.code < r.code;
}
bool cnt_descending(Lmtf l, Lmtf r)
{
    return l.cnt > r.cnt;
}

#endif //_TYPES_H

#include "types.h"

bool code_ascending(Lmtf l, Lmtf r)
{
    return l.code < r.code;
}
bool cnt_descending(Lmtf l, Lmtf r)
{
    return l.cnt > r.cnt;
}
#include "NumericalUtils.h"
#include <math.h>

NumericalUtils::NumericalUtils()
{

}

bool NumericalUtils::compareDoubles(const double &x, const double &y, const double &epsilon)
{
    bool ret = fabs(x - y) <= epsilon;
    return ret;
 }

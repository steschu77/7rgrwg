#pragma once

#include "Filter.h"

namespace filter {

class Displace: public Filter
{
public:
  Displace(Filter* pf0, Filter* pf1, Filter* pf2, Filter* pf3): Filter(pf0, pf1, pf2, pf3) {}

  double GetValue (double x, double y, double z) const
  {
    double xDisplace = x + (_pSource[1]->GetValue(x, y, z));
    double yDisplace = y + (_pSource[2]->GetValue(x, y, z));
    double zDisplace = z + (_pSource[3]->GetValue(x, y, z));

    return _pSource[0]->GetValue(xDisplace, yDisplace, zDisplace);
  }
};

}

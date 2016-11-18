#pragma once

#include "Filter.h"
#include "noise/Interpolation.h"

namespace filter {

class Blend: public Filter
{
public:
  Blend(Filter* pf0, Filter* pf1, Filter* pf2): Filter(pf0, pf1, pf2) {}

  double GetValue (double x, double y, double z) const
  {
    double v0 = _pSource[0]->GetValue(x, y, z);
    double v1 = _pSource[1]->GetValue(x, y, z);
    double alpha = (_pSource[2]->GetValue(x, y, z) + 1.0) * 0.5;
    return noise::LinearInterp(v0, v1, alpha);
  }
};

}

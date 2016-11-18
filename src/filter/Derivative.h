#pragma once

#include "Filter.h"

namespace filter {

class DerivativeX: public Filter
{
public:
  DerivativeX(Filter* pf0, double d): Filter(pf0), _d(d) {}

  double GetValue(double x, double y, double z) const
  {
    double x0 = _pSource[0]->GetValue(x-_d, y, z);
    double x1 = _pSource[0]->GetValue(x+_d, y, z);
    return (x1 - x0) / (2*_d);
  }
  
  double _d;
};

}

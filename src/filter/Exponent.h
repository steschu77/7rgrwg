#pragma once

#include "Filter.h"

namespace filter {

class Exponent: public Filter
{
public:
  Exponent(Filter* pf0, double e): Filter(pf0), _e(e) {}

  double GetValue(double x, double y, double z) const {
    double Val = _pSource[0]->GetValue(x, y, z);
    return (pow(fabs((Val + 1.0) / 2.0), _e) * 2.0 - 1.0);
  }
  
  double _e;
};

}

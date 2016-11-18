#pragma once

#include "Filter.h"

namespace filter {

class Clamp: public Filter
{
public:
  Clamp(Filter* pf0, double Lower=-1.0, double Upper=1.0) : Filter(pf0), _LowerBound(Lower), _UpperBound(Upper) {}

  virtual double GetValue(double x, double y, double z) const {
    double Value = _pSource[0]->GetValue(x, y, z);
    return std::min(std::max(Value, _LowerBound), _UpperBound);
  }

protected:
  double _LowerBound;
  double _UpperBound;
};

}

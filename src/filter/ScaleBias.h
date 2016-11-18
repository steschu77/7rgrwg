#pragma once

#include "Filter.h"

namespace filter {

class ScaleBias: public Filter
{
public:
  ScaleBias(Filter* pf0, double s=1, double b=0): Filter(pf0), _Bias(b), _Scale(s) {}

  double GetValue (double x, double y, double z) const {
    return _pSource[0]->GetValue(x, y, z) * _Scale + _Bias;
  }
  
protected:
  double _Bias;
  double _Scale;
};

}

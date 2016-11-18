#pragma once

#include "Filter.h"

namespace filter {

class Quantizer: public Filter
{
public:
  Quantizer(Filter* pf0, double q): Filter(pf0), _Quantizer(q) {}

  double GetValue(double x, double y, double z) const {
    double d = _pSource[0]->GetValue(x, y, z);
    return d - std::fmod(d, _Quantizer);
  }

private:
  double _Quantizer;
};

}

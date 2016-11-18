#pragma once

#include "Filter.h"

namespace filter {

class Abs: public Filter
{
public:
  Abs(Filter* pf0): Filter(pf0) {}

  double GetValue (double x, double y, double z) const {
    return fabs(_pSource[0]->GetValue(x, y, z));
  }
};

}

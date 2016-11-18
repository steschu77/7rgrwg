#pragma once

#include "Filter.h"

namespace filter {

class Invert: public Filter
{
public:
  Invert(Filter* pf0): Filter(pf0) {}

  double GetValue (double x, double y, double z) const {
    return -_pSource[0]->GetValue(x, y, z);
  }
};

}

#pragma once

#include "Filter.h"

namespace filter {

class Min: public Filter
{
public:
  Min(Filter* pf0, Filter* pf1): Filter(pf0, pf1) {}

  double GetValue (double x, double y, double z) const {
    return std::min(_pSource[0]->GetValue(x, y, z),
                    _pSource[1]->GetValue(x, y, z));
  }
};

}


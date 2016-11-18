#pragma once

#include "Filter.h"

namespace filter {

struct ScalePoint: public Filter
{
  ScalePoint(Filter* pf0, double x, double y, double z): Filter(pf0), _xScale(x), _yScale(y), _zScale(z) {}

  double GetValue (double x, double y, double z) const
  {
    return _pSource[0]->GetValue (x * _xScale, y * _yScale, z * _zScale);
  }

protected:
  double _xScale;
  double _yScale;
  double _zScale;
};

}

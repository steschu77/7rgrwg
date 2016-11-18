#pragma once

#include "Filter.h"

namespace filter {

class Curve: public Filter
{
public:
  Curve(Filter* pf0, const double* pPoints, size_t cPoints);

  virtual double GetValue (double x, double y, double z) const;

protected:
  typedef std::map<double, double> ControlPoints;
  ControlPoints _ControlPoints;
};

}

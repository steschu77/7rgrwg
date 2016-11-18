#include "main.h"

#include "Curve.h"
#include "noise/Interpolation.h"

// ----------------------------------------------------------------------------
filter::Curve::Curve(Filter* pf0, const double* pPoints, size_t cPoints)
: Filter(pf0)
{
  for (size_t i = 0; i < cPoints; i++) {
    _ControlPoints[pPoints[2 * i]] = pPoints[2 * i + 1];
  }
}

// ----------------------------------------------------------------------------
double filter::Curve::GetValue(double x, double y, double z) const
{
  double sourceModuleValue = _pSource[0]->GetValue(x, y, z);

  ControlPoints::const_iterator i2 = _ControlPoints.lower_bound(sourceModuleValue);

  ControlPoints::const_iterator i1 = i2;
  if (i1 != _ControlPoints.begin()) i1--;

  ControlPoints::const_iterator i0 = i1;
  if (i0 != _ControlPoints.begin()) i0--;

  ControlPoints::const_iterator i3 = i2;
  if (i3 != _ControlPoints.end())   i3++;

  double p0 = i0->second;
  double p1 = i1->second;
  double p2 = i2->second;
  double p3 = i3->second;

  // Compute the alpha value used for cubic interpolation.
  double alpha = (sourceModuleValue - p1) / (p2 - p1);

  // Now perform the cubic interpolation given the alpha value.
  return noise::CubicInterp(p0, p1, p2, p3, alpha);
}

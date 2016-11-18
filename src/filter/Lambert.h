#pragma once

#include "Filter.h"

namespace filter {

class Lambert: public Filter
{
public:
  Lambert(Filter* pf0, Filter* pf1, double p, double q): Filter(pf0, pf1), _Ps(p), _Qs(q)
  {
    _s = sqrt(_Ps*_Ps + _Qs*_Qs + 1.0);
  }

  double GetValue(double x, double y, double z) const
  {
    double p = _pSource[0]->GetValue(x, y, z);
    double q = _pSource[1]->GetValue(x, y, z);
    
    double pq = sqrt(p*p + q*q + 1.0);
    return (_Ps*p + _Qs*q + 1) / (_s * pq);
  }

private:
  double _Ps;
  double _Qs;
  double _s;
};

}

#pragma once

#include "Filter.h"

namespace filter {

class Checkerboard: public Filter
{
public:
  double Checkerboard::GetValue(double x, double y, double z) const
  {
    int ix = (int)floor(x);
    int iy = (int)floor(y);
    int iz = (int)floor(z);
    return ((ix & 1) ^ (iy & 1) ^ (iz & 1)) ? -1.0: 1.0;
  }
};

}

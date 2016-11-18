#pragma once

#include "Filter.h"

namespace filter {

class Billow: public Filter
{
public:
  Billow(double f = 1.0, double l = 2.0, double p = 0.5, int o = 6, int s = 0);

  virtual double GetValue(double x, double y, double z) const;

protected:
  double _Frequency;
  double _Lacunarity;
  double _Persistence;
  int _cOctaves;
  int _Seed;
};

}

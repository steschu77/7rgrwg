#pragma once

#include "Filter.h"

namespace filter {

class WorleyNoise: public Filter
{
public:
  WorleyNoise(int count);

  virtual double GetValue(double x, double y, double z) const;
  
protected:
  struct point_t
  {
    point_t(double x, double y, double z): x(x), y(y), z(z) {}
    double x, y, z;
  };

  typedef std::vector<point_t> points_t;

  points_t _Points;

  static double distance(const point_t& x0, const point_t& x1);
};

}

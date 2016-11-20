#include "main.h"

#include "WorleyNoise.h"
#include "noise/NoiseGen.h"

// ----------------------------------------------------------------------------
static float genNoise()
{
  return (float)rand() / 32767.0f;
}

// ----------------------------------------------------------------------------
filter::WorleyNoise::WorleyNoise(int count)
{
  for (int i = 0; i < count; i++)
  {
    float x = genNoise() * 0.9f + .05f;
    float y = genNoise() * 0.9f + .05f;
    _Points.push_back(point_t(x, y, 0));
  }
}

// ----------------------------------------------------------------------------
double filter::WorleyNoise::GetValue(double x, double y, double z) const
{
  point_t pt(x, y, z);

  double d0 = distance(_Points[0], pt);
  double min0 = d0;
  double min1 = d0;
  int i0 = 0;
  int i1 = 0;

  for (size_t i = 1; i < _Points.size(); ++i) {
    double d = distance(_Points[i], pt);
    if (d < min0) {
      min1 = min0;
      min0 = d;
      i0 = i;
    } else if (d < min1) {
      min1 = d;
      i1 = i;
    }
  }

  double dMin0 = sqrt(min0);
  double dMin1 = sqrt(min1);
  //double dMax = sqrt(distance(_Points[i0], _Points[i1]));

  return dMin1 - dMin0;
}

// ----------------------------------------------------------------------------
double filter::WorleyNoise::distance(const point_t& x0, const point_t& x1)
{
  std::function<double(double)> sqr = [](double x) { return x*x; };
  return sqr(x0.x-x1.x) + sqr(x0.y-x1.y) + sqr(x0.z-x1.z);
}

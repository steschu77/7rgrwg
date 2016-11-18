#pragma once

namespace noise {

// The alpha value should range from 0.0 to 1.0. If the alpha value is 0.0, this
// function returns n1. If the alpha value is 1.0, this function returns n2.
inline double CubicInterp(double n0, double n1, double n2, double n3, double a)
{
	double p = (n3 - n2) - (n0 - n1);
	double q = (n0 - n1) - p;
	double r = n2 - n0;
	double s = n1;
	return p * a * a * a + q * a * a + r * a + s;
}

// The alpha value should range from 0.0 to 1.0. If the alpha value is 0.0, this
// function returns n0. If the alpha value is 1.0, this function returns n1.
inline double LinearInterp(double n0, double n1, double a)
{
  return ((1.0 - a) * n0) + (a * n1);
}

// The derivative of a cubic S-curve is zero at a = 0.0 and a = 1.0
inline double SCurve3(double a)
{
  return (a * a * (3.0 - 2.0 * a));
}

// The first derivative of a quintic S-curve is zero at a = 0.0 and a = 1.0
// The second derivative of a quintic S-curve is zero at a = 0.0 and a = 1.0
inline double SCurve5(double a)
{
  double a3 = a * a * a;
  double a4 = a3 * a;
  double a5 = a4 * a;
  return (6.0 * a5) - (15.0 * a4) + (10.0 * a3);
}

} // noise

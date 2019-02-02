#include "main.h"
#include "vector2.h"

// ============================================================================
const float x3d::epsilon = 0.00001f;

// ============================================================================
x3d::vector2::vector2()
: m{ 0, 0 }
{
}

// ----------------------------------------------------------------------------
x3d::vector2::vector2(const vector2 &v)
: m{ v.m[0], v.m[1] }
{
}

// ----------------------------------------------------------------------------
x3d::vector2::vector2(float u0, float u1)
: m{ u0, u1 }
{
}

// ----------------------------------------------------------------------------
x3d::vector2::vector2(const float u[2])
: m{ u[0], u[1] }
{
}

// ----------------------------------------------------------------------------
void x3d::vector2::get(float a[3]) const
{
  a[0] = m[0];
  a[1] = m[1];
}

// ----------------------------------------------------------------------------
void x3d::vector2::put(const float a[3])
{
  m[0] = a[0];
  m[1] = a[1];
}

// ----------------------------------------------------------------------------
float x3d::vector2::u0() const
{
  return m[0];
}

// ----------------------------------------------------------------------------
float x3d::vector2::u1() const
{
  return m[1];
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::operator = (const x3d::vector2 & v)
{
  m[0] = v.m[0];
  m[1] = v.m[1];
  return *this;
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::operator - () const
{
  return x3d::vector2(-m[0], -m[1]);
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::operator += (const x3d::vector2 & v)
{
  m[0] += v.m[0];
  m[1] += v.m[1];
  return *this;
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::operator -= (const x3d::vector2 & v)
{
  m[0] -= v.m[0];
  m[1] -= v.m[1];
  return *this;
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::operator + (const x3d::vector2 & v) const
{
  return vector2(
    m[0] + v.m[0],
    m[1] + v.m[1]);
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::operator - (const x3d::vector2 & v) const
{
  return x3d::vector2(
    m[0] - v.m[0],
    m[1] - v.m[1]);
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::operator * (float f) const
{
  return x3d::vector2(m[0] * f, m[1] * f);
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::operator / (float f) const
{
  return x3d::vector2(m[0] / f, m[1] / f);
}

// ----------------------------------------------------------------------------
float x3d::vector2::operator * (const x3d::vector2 & v) const
{
  return (m[0] * v.m[0]) + (m[1] * v.m[1]);
}

// ----------------------------------------------------------------------------
bool x3d::vector2::operator == (const x3d::vector2 & v) const
{
  return (m[0] == v.m[0]) && (m[1] == v.m[1]);
}

// ----------------------------------------------------------------------------
bool x3d::vector2::operator != (const x3d::vector2 & v) const
{
  return (m[0] != v.m[0]) || (m[1] != v.m[1]);
}

// ----------------------------------------------------------------------------
float x3d::vector2::length2() const
{
  return m[0] * m[0] + m[1] * m[1];
}

// ----------------------------------------------------------------------------
float x3d::vector2::length() const
{
  return sqrtf(length2());
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::norm()
{
  float l = length();
  if (l < epsilon) {
    return x3d::vector2(0.0f, 0.0f);
  } else {
    return x3d::vector2(m[0] / l, m[1] / l);
  }
}

// ----------------------------------------------------------------------------
// Get the skew vector such that dot(perpendicular, other) == cross(vec, other)
x3d::vector2 x3d::vector2::perpendicular()
{
  return x3d::vector2(-m[1], m[0]);
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::vector2::abs()
{
  return x3d::vector2(fabs(m[0]), fabs(m[1]));
}

/*
// ----------------------------------------------------------------------------
vector2 x3d::vector2::operator *= (const x3d::matrix2x2 &m)
{
  return *this;
}
*/

// ----------------------------------------------------------------------------
x3d::vector2 x3d::operator * (float f, const x3d::vector2& v)
{
  return x3d::vector2(f*v.m[0], f*v.m[1]);
}

// ----------------------------------------------------------------------------
float x3d::dot(const x3d::vector2& a, const x3d::vector2& b)
{
  return (a.m[0] * b.m[0]) + (a.m[1] * b.m[1]);
}

// ----------------------------------------------------------------------------
// Two crossed vectors return a scalar
// * area of the parallelogram of the 2 vectors
// * magnitude of the Z vector of 3D cross product
// * sign says a rotates CW or CCW to b
float x3d::cross(const x3d::vector2& a, const x3d::vector2& b)
{
  return a.m[0] * b.m[1] - a.m[1] * b.m[0];
}

// ----------------------------------------------------------------------------
// More exotic (but necessary) forms of the cross product
// with a vector a and scalar s, both returning a vector
x3d::vector2 x3d::cross(const x3d::vector2& a, float s)
{
  return x3d::vector2(s * a.m[1], -s * a.m[0]);
}

// ----------------------------------------------------------------------------
x3d::vector2 x3d::cross(float s, const x3d::vector2& a)
{
  return x3d::vector2(-s * a.m[1], s * a.m[0]);
}

// ----------------------------------------------------------------------------
auto x3d::absDiff(const x3d::vector2& v0, const x3d::vector2& v1) -> x3d::vector2
{
  float d0 = std::max(v1.u0() - v0.u0(), v0.u0() - v1.u0());
  float d1 = std::max(v1.u1() - v0.u1(), v0.u1() - v1.u1());
  return x3d::vector2(d0, d1);
}

// ----------------------------------------------------------------------------
auto x3d::bezierPoint2(float t, const x3d::vector2* p) -> x3d::vector2
{
  float u = 1 - t;
  return u*u * p[0] + 2 * u*t * p[1] + t*t * p[2];
}

// ----------------------------------------------------------------------------
auto x3d::bezierPoint3(float t, const x3d::vector2* p) -> x3d::vector2
{
  float u = 1 - t;
  return u*u*u * p[0] + 3*u*u*t * p[1] + 3*u*t*t * p[1] + t*t*t * p[2];
}

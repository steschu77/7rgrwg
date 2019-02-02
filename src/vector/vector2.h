#pragma once

namespace x3d {

extern const float epsilon;

// ============================================================================
struct vector2
// ----------------------------------------------------------------------------
{
  vector2();
  vector2(const vector2 &v);

  vector2(float u0, float u1);
  vector2(const float u[2]);

  void get(float a[2]) const;
  void put(const float a[2]);

  float u0() const;
  float u1() const;

  vector2 operator = (const vector2 & v);
  vector2 operator - () const;
  vector2 operator += (const vector2 & v);
  vector2 operator -= (const vector2 & v);
  vector2 operator +  (const vector2 & v) const;
  vector2 operator -  (const vector2 & v) const;

  vector2 operator *  (float f) const;
  vector2 operator /  (float f) const;

  float  operator *  (const vector2 & v) const;

  bool   operator == (const vector2 & v) const;
  bool   operator != (const vector2 & v) const;

  float length2() const;
  float length() const;

  vector2 norm();
  vector2 perpendicular();

  vector2 abs();

  float m[2];
};

// ============================================================================
vector2 operator *  (float f, const vector2& v);

// ============================================================================
struct matrix2x2
// ----------------------------------------------------------------------------
{
  matrix2x2(float x = 0);
  matrix2x2(float x00, float x01,
            float x10, float x11);
  matrix2x2(const float x[2][2]);

  matrix2x2 transpose() const;
  matrix2x2 inverse() const;
  vector2 solve(const vector2& v) const;

  float m[2][2];
};

vector2 operator * (const vector2& v, const matrix2x2& m);
vector2 operator * (const matrix2x2& m, const vector2& v);
matrix2x2 operator * (const matrix2x2& m1, const matrix2x2& m2);

// ============================================================================
matrix2x2 getZeroMatrix2x2();
matrix2x2 getIdentityMatrix2x2();

// ============================================================================
vector2 rotate2(const vector2& v, float deg);
matrix2x2 translate2x2(const vector2 & v);
matrix2x2 rotate2x2(float deg);

float dot(const vector2& v0, const vector2& v1);
float cross(const vector2& v0, const vector2& v1);
vector2 cross(const vector2& v, float s);
vector2 cross(float s, const vector2& v);

auto absDiff(const vector2& v0, const vector2& v1) -> vector2;
auto bezierPoint2(float t, const vector2* p) -> vector2;
auto bezierPoint3(float t, const vector2* p) -> vector2;

} // namespace x3d

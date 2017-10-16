#pragma once

namespace graph {

// ============================================================================
struct point_t
{
  point_t();
  point_t(double x, double y);

  double x;
  double y;
};

// ============================================================================
struct unique_point_t
{
  unique_point_t();
  unique_point_t(int id, double x, double y);

  int id;
  point_t pt;
};

using corner_t = unique_point_t;
using center_t = unique_point_t;

// ============================================================================
struct segment_t
{
  segment_t();
  segment_t(int id, const corner_t& p0, const corner_t& p1, const center_t& c0, const center_t& c1);

  int id;
  corner_t p0;
  corner_t p1;
  center_t c0;
  center_t c1;
};

// ============================================================================
typedef std::vector<center_t> centers_t;
typedef std::vector<segment_t> segments_t;

point_t operator + (const point_t& p0, const point_t& p1);
point_t operator - (const point_t& p0, const point_t& p1);
point_t operator * (double s, const point_t& p);
point_t operator * (const point_t& p, double s);

double length(const point_t& p);
point_t perpendicular(const point_t& p);
point_t normalize(const point_t& p, double x = 1.0);

// ============================================================================
auto clipLine(point_t& p0, point_t& p1, double cx, double cy) -> int;
auto bezierPoint(double t, const graph::point_t* p) -> point_t;
void voronoi(const centers_t& pts, segments_t& segs);

}

// ============================================================================
inline auto graph::operator + (const point_t& p0, const point_t& p1) -> point_t
{
  return point_t(p0.x+p1.x, p0.y+p1.y);
}

// ============================================================================
inline auto graph::operator - (const point_t& p0, const point_t& p1) -> point_t
{
  return point_t(p0.x-p1.x, p0.y-p1.y);
}

// ============================================================================
inline auto graph::operator * (double s, const point_t& p) -> point_t
{
  return point_t(s*p.x, s*p.y);
}

// ============================================================================
inline auto graph::operator * (const point_t& p, double s) -> point_t
{
  return point_t(s*p.x, s*p.y);
}

// ============================================================================
inline auto graph::length(const point_t& p) -> double
{
  return sqrt(p.x*p.x + p.y*p.y);
}

// ============================================================================
inline auto graph::perpendicular(const point_t& p) -> point_t
{
  return point_t(p.y, -p.x);
}

// ============================================================================
inline auto graph::normalize(const point_t& p, double x) -> point_t
{
  double d = x / length(p);
  return p * d;
}

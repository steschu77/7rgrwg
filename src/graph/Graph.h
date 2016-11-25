#pragma once

namespace graph {

// ============================================================================
struct center_t
{
  center_t();
  center_t(int id, double x, double y);

  int id;
  double x;
  double y;
};

// ============================================================================
struct point_t
{
  point_t();
  point_t(int id, double x, double y);

  int id;
  double x;
  double y;
};

// ============================================================================
struct segment_t
{
  segment_t();
  segment_t(int id, const point_t& p0, const point_t& p1, const center_t& c0, const center_t& c1);

  int id;
  point_t p0;
  point_t p1;
  center_t c0;
  center_t c1;
};

// ============================================================================
typedef std::vector<center_t> centers_t;
typedef std::vector<segment_t> segments_t;

// ============================================================================
int clipLine(point_t& p0, point_t& p1, double cx, double cy);
void voronoi(const centers_t& pts, segments_t& segs);

}

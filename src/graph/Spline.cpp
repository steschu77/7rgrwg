#include "main.h"
#include "graph/Graph.h"

// ============================================================================
auto graph::bezierPoint(double t, const graph::point_t* p) -> point_t
{
  double u  = 1 - t;
  double uu = u * u;
  double tt = t * t;

  return uu * p[0] + 2*u*t * p[1] + tt * p[2];
}

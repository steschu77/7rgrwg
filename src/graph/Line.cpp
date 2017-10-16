#include "main.h"
#include "graph/Graph.h"

const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

// ============================================================================
template <typename T>
static int getClipCode(T x, T y, T cx, T cy)
{
  return ((x < 0) << 0) | ((x > cx) << 1)
       | ((y < 0) << 2) | ((y > cy) << 3);
}

// ============================================================================
int graph::clipLine(point_t& p0, point_t& p1, double cx, double cy)
{
  double x0 = p0.x;
  double y0 = p0.y;
  double x1 = p1.x;
  double y1 = p1.y;

  int outcode0 = getClipCode(x0, y0, cx, cy);
  int outcode1 = getClipCode(x1, y1, cx, cy);

  int accept = 0;

  for (;;)
  {
    if (!(outcode0 | outcode1)) {
      accept |= 1;
      break;
    } else if (outcode0 & outcode1) {
      break;
    } else {
      double x = 0;
      double y = 0;

      int outcodeOut = outcode0 ? outcode0 : outcode1;

      if (outcodeOut & TOP) {
        x = x0 + (x1 - x0) * (cy - y0) / (y1 - y0);
        y = cy;
      } else if (outcodeOut & BOTTOM) {
        x = x0 + (x1 - x0) * (0  - y0) / (y1 - y0);
        y = 0;
      } else if (outcodeOut & RIGHT) {
        y = y0 + (y1 - y0) * (cx - x0) / (x1 - x0);
        x = cx;
      } else if (outcodeOut & LEFT) {
        y = y0 + (y1 - y0) * (0  - x0) / (x1 - x0);
        x = 0;
      }

      if (outcodeOut == outcode0) {
        x0 = x;
        y0 = y;
        outcode0 = getClipCode(x0, y0, cx, cy);
        accept |= 2;
      } else {
        x1 = x;
        y1 = y;
        outcode1 = getClipCode(x1, y1, cx, cy);
        accept |= 4;
      }
    }
  }
  if ((accept & 1) != 0) {
    p0.x = x0;
    p0.y = y0;
    p1.x = x1;
    p1.y = y1;
  }
  return accept;
}

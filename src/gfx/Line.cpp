#include "main.h"
#include "gfx/gfx.h"

// ============================================================================
template <typename Func>
static void drawLineLoop(int u0, int v0, int du, int dv, Func func)
{
  int d = (dv << 16) / du;
  int cvar = 1 << 15;

  int u = u0;
  int v = v0;

  while (u < u0+du)
  {
    func(u, v);
    cvar += d;
    u += 1;
    v += (cvar >> 16);
    cvar &= 0xffff;
  }
}

// ============================================================================
void gfx::drawLine(image_t& img, int x0, int y0, int x1, int y1, uint32_t color)
{
  int dx = x1 - x0;
  int dy = y1 - y0;

  if ((dx|dy) == 0) {
    return;
  }

  auto drawXY = [&](int x, int y) { putPixel(img, x, y, color); };
  auto drawYX = [&](int y, int x) { putPixel(img, x, y, color); };

  if (abs(dx) >= abs(dy))
  {
    if (x0 < x1) {
      drawLineLoop(x0, y0,  dx,  dy, drawXY);
    } else {
      drawLineLoop(x1, y1, -dx, -dy, drawXY);
    }
  } else {
    if (y0 < y1) {
      drawLineLoop(y0, x0,  dy,  dx, drawYX);
    } else {
      drawLineLoop(y1, x1, -dy, -dx, drawYX);
    }
  }
}

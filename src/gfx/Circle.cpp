#include "main.h"
#include "gfx/gfx.h"

// ============================================================================
// Bresenham's Circle Algorithm
void gfx::drawCircle(gfx::image_t& img, int x, int y, int r, uint32_t color)
{
  int dx = 0;
  int dy = r;

  putPixel(img, x, y+r, color);
  putPixel(img, x, y-r, color);
  putPixel(img, x+r, y, color);
  putPixel(img, x-r, y, color);
  
  int d = 3 - 2*r;
  
  while (dx <= dy)
  {
    if (d <= 0) {
      d += 4*dx + 6;
    } else {
      d += 4*(dx-dy) + 10;
      dy--;
    }

    dx++;

    putPixel(img, x+dx, y+dy, color);
    putPixel(img, x-dx, y+dy, color);
    putPixel(img, x+dx, y-dy, color);
    putPixel(img, x-dx, y-dy, color);
    putPixel(img, x+dy, y+dx, color);
    putPixel(img, x-dy, y+dx, color);
    putPixel(img, x+dy, y-dx, color);
    putPixel(img, x-dy, y-dx, color);
  }
}

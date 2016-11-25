#include "main.h"
#include "gfx/gfx.h"

// ----------------------------------------------------------------------------
static void calcSlope(gfx::canvas_t& canvas, const gfx::point_t& p0, const gfx::point_t& p1)
{
  unsigned y0 = p0.y;
  unsigned y1 = p1.y;

  if (y0 == y1)
  {
    if (canvas.yMin > y0) { canvas.yMin = y0; }
    if (canvas.yMax < y0) { canvas.yMax = y0; }
    return;
  }

  unsigned x0 = (p0.x << 16) + 0x8000;
  unsigned x1 = (p1.x << 16) + 0x8000;
  int dx = x1 - x0;
  int dy = y1 - y0;
  int xInc = dx / dy;
  
  if (y0 < y1)
  {
    if (canvas.yMin > y0) { canvas.yMin = y0; }
    for (; y0 < y1; y0++, x0+=xInc) {
      canvas.edge[0][y0] = x0 >> 16;
    }
  }
  else
  {
    if (canvas.yMax < y0) { canvas.yMax = y0; }
    for (; y0 > y1; y1++, x1+=xInc) {
      canvas.edge[1][y1] = x1 >> 16;
    }
  }
}

// ----------------------------------------------------------------------------
void gfx::drawPolyStrip(image_t& img, canvas_t& canvas,
  const point_t* pVerts, const uint32_t* pColor, size_t cFaces)
{
  calcSlope(canvas, pVerts[0], pVerts[1]);

  for (size_t i = 0; i < cFaces; i++)
  {
    canvas.yMin = canvas.yMax = pVerts[i].y;
    calcSlope(canvas, pVerts[i+2], pVerts[i]);
    calcSlope(canvas, pVerts[i+1], pVerts[i+2]);

    for (int y = canvas.yMin; y < canvas.yMax; y++)
    {
      int x0 = canvas.edge[0][y];
      int x1 = canvas.edge[1][y];
      int xMin = std::min(x0, x1);
      int xMax = std::max(x0, x1);

      for (int x = xMin; x < xMax; x++) {
        putPixel(img, x, y, pColor[i]);
      }
    }
  }
}

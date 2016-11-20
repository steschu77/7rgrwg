#include "main.h"
#include "gen.h"

#include "gfx/gfx.h"
#include "filter/Filters.h"

// ============================================================================
void putPixel(gfx::image_t& img, unsigned x, unsigned y, uint8_t color)
{
  size_t ofs = x + y * img.stride.p[0];
  img.data[0][ofs] = color;
}

// ============================================================================
void gen::generateHeightMap(world::world_t* pWorld)
{
  filter::Perlin perlin(1.0, 2.0, 0.5, 3);
  filter::RidgedMulti ridged(1.0, 2.0, 3);

  unsigned cx = pWorld->cxMeters();
  unsigned cy = pWorld->cyMeters();

  gfx::imagegeo_t geo(gfx::cfY8, cx, cy);
  gfx::image_t img;
  img.stride.p[0] = cx;
  img.data[0] = new uint8_t [img.stride.p[0] * geo.cy];

  for (unsigned y = 0; y < cy; ++y) {
    for (unsigned x = 0; x < cx; ++x) {
      //uint8_t color = static_cast<uint8_t>(std::min(std::max((perlin.GetValue(x/256.0, y/256.0, 0) + 1.0) * 128.0, 0.0), 255.0));
      float a = std::min(std::max((ridged.GetValue(x/512.0, y/512.0, 0) + 1.0) * 64.0f, 0.0), 255.0);
      pWorld->height[y][x] = a;
      uint8_t color = static_cast<uint8_t>(a);
      putPixel(img, x, y, color);
    }
  }

  gfx::saveBMPFile("g:\\Temp\\test.bmp", geo, img, true);

  delete img.data[0];
}

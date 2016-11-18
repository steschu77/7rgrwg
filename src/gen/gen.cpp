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
void gen::generateHeightMap()
{
  filter::Perlin perlin(1.0, 2.0, 0.5, 3);

  gfx::imagegeo_t geo(gfx::cfY8, 1024u, 1024u);
  gfx::image_t img;
  img.stride.p[0] = 1024;
  img.data[0] = new uint8_t [img.stride.p[0] * geo.cy];

  for (unsigned y = 0; y < geo.cy; ++y) {
    for (unsigned x = 0; x < geo.cx; ++x) {
      uint8_t color = static_cast<uint8_t>(std::min(std::max((perlin.GetValue(x/256.0, y/256.0, 0) + 1.0) * 128.0, 0.0), 255.0));
      putPixel(img, x, y, color);
    }
  }

  gfx::saveBMPFile("g:\\Temp\\test.bmp", geo, img, true);

  delete img.data[0];
}

#include "main.h"
#include "gen.h"

#include "gfx/gfx.h"
#include "filter/Filters.h"
#include "7dtd/BlockIds.h"

// ============================================================================
void putPixel(gfx::image_t& img, unsigned x, unsigned y, uint8_t color)
{
  size_t ofs = x + y * img.stride.p[0];
  img.data[0][ofs] = color;
}

// ============================================================================
void gen::generateHeightMap(world::world_t* pWorld)
{
  filter::Perlin perlinSrc(1.0, 2.0, 0.5, 3);
  filter::ScaleBias perlin(&perlinSrc, 0.5, 0.5);
  filter::RidgedMulti ridged(1.0, 2.0, 3);
  filter::WorleyNoise worley(5000);

  unsigned cx = pWorld->cxMeters();
  unsigned cy = pWorld->cyMeters();

  gfx::imagegeo_t geo(gfx::cfY8, cx, cy);
  gfx::image_t img;
  img.stride.p[0] = cx;
  img.data[0] = new uint8_t [img.stride.p[0] * geo.cy];

  for (unsigned y = 0; y < cy; ++y) {
    printf("generate %d\r", y);
    for (unsigned x = 0; x < cx; ++x) {
      float c = perlin.GetValue(x/256.0, y/256.0, 0);
      //c = std::min(std::max((c*c*c*c) * 128.0, 0.0), 255.0);
      float a = (ridged.GetValue(x/512.0, y/512.0, 0) + 1.0) * 64.0f;
      float b = worley.GetValue(x/1024.0, y/1024.0, 0) * 1024.0f;

      float d = (c > 0.95) ? a+b : a;
      uint32_t id = (c > 0.95) ? idIronOre : (c < 0.1 ? idGrass : idSand);
      d = std::min(std::max(d, 0.0f), 255.0f);

      pWorld->height[y][x] = d;
      pWorld->block[y][x] = id;

      uint8_t color = static_cast<uint8_t>(d);
      putPixel(img, x, y, color);
    }
  }

  gfx::saveBMPFile("g:\\Temp\\test.bmp", geo, img, true);
  printf("done. map saved.\n");

  delete img.data[0];
}

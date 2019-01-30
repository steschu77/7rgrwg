#include "main.h"
#include <list>

#include "RoadGen.h"

#include "gfx/gfx.h"
#include "filter/Filters.h"
#include "world/BlockIds.h"

static int cCities = 4;

namespace noise {
  extern double gRandomVectors[256 * 4];
}

// ============================================================================
template <typename T>
T randn(const T& n)
{
  return (n * rand()) / RAND_MAX;
}

// ============================================================================
struct road_t
{
  std::string name;
  int cityA;
  int cityB;
};

// ============================================================================
struct city_t
{
  std::string name;
  int xCenter;
  int yCenter;
};

// ============================================================================
struct road_gen_t
{
  std::vector<city_t> cities;
  std::vector<road_t> roads;
};

// ============================================================================
void gen::generateRoadMap(world::world_t* pWorld)
{
  filter::Perlin perlinSrc(1.0, 2.0, 0.5, 3);
  filter::ScaleBias perlin(&perlinSrc, 0.5, 0.5);
  filter::RidgedMulti ridged(1.0, 2.0, 3);
  filter::WorleyNoise worley(5000);
  filter::Perlin scatter(32.0, 2.1, 0.6, 4);

  unsigned cx = 1024;
  unsigned cy = 1024;

  gfx::imagegeo_t geo(gfx::cfY8, cx, cy);
  gfx::image_t img;
  img.stride.p[0] = cx;
  img.data[0] = new uint8_t [img.stride.p[0] * geo.cy];

  for (unsigned y = 0; y < cy; ++y) {
    printf("generate %d\r", y);
    for (unsigned x = 0; x < cx; ++x)
    {
      float c = perlin.GetValue(x/256.0, y/256.0, 0);
      //c = std::min(std::max((c*c*c*c) * 128.0, 0.0), 255.0);
      float a = (ridged.GetValue(x/512.0, y/512.0, 0) + 1.0) * 64.0f;
      //float b = worley.GetValue(x/1024.0, y/1024.0, 0) * 1024.0f;
      float s = scatter.GetValue(x/256.0, y/256.0, 0) * 0.5f;

      float b = 0;
      float d = (c > 0.95) ? a+b : a+s;
      uint32_t id = (c > 0.95) ? idIronOre : (c < 0.1 ? idGrass : idSand);
      d = std::min(std::max(d, 0.0f), 255.0f);

      uint8_t color = static_cast<uint8_t>(d);
      gfx::putPixel(img, x, y, color);
    }
  }

  printf("Generate cities...\r");
  graph::centers_t pts;
  for (int i = 0; i < cCities; i++)
  {
    double mx = (i & 1) * cx * 0.5;
    double my = (i >> 1) * cy * 0.5;

    double x = mx + randn(cx/2);
    double y = my + randn(cy/2);
    pts.push_back(graph::center_t(i, x, y));
  }
  /*
  graph::segments_t segs;
  voronoi(pts, segs);

  pts.clear();
  easePoints(segs, pts, cCities - 1, cx, cy);

  segs.clear();
  voronoi(pts, segs);

  pts.clear();
  easePoints(segs, pts, cCities - 1, cx, cy);

  segs.clear();
  voronoi(pts, segs);
  */
  //map_t map;
  //createMap(segs, pts, map, *pWorld);

  uint8_t color = 0xf0;
  for (const auto& pt : pts) {
    gfx::drawLine(img, (int)pt.pt.x - 2, (int)pt.pt.y, (int)pt.pt.x + 3, (int)pt.pt.y, color);
    gfx::drawLine(img, (int)pt.pt.x, (int)pt.pt.y - 2, (int)pt.pt.x, (int)pt.pt.y + 3, color);
  }

  gfx::saveBMPFile("g:\\Temp\\roads.bmp", geo, img, true);


  printf("done. map saved.\n");

  delete img.data[0];
}

#include "main.h"
#include <list>

#include "RoadGen.h"

#include "gfx/gfx.h"
#include "filter/Filters.h"
#include "world/BlockIds.h"
#include "graph/Graph.h"
#include "vector/vector2.h"

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
struct coord_t
{
  int x;
  int y;
};

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

using Path = std::vector<x3d::vector2>;

// ----------------------------------------------------------------------------
float length(const Path& p)
{
  float len = 0.0f;
  Path::const_iterator j = p.cbegin();
  Path::const_iterator i = j;

  while (++j != p.cend())
  {
    x3d::vector2 d = *j - *i;
    len += d.length();
    i = j;
  }

  return len;
}

// ----------------------------------------------------------------------------
float evaluate(const Path& p, float* height, int cx, int cy)
{
  std::vector<coord_t> coords;
  auto fn = [&](int x, int y) { coords.push_back({ x, y }); };

  Path::const_iterator j = p.cbegin();
  Path::const_iterator i = j;

  while (++j != p.cend()) {
    gfx::drawLine(i->u0(), i->u1(), j->u0(), j->u1(), fn);
    i = j;
  }

  float len = 0;
  float h0 = 0;
  bool first = true;
  for (auto c : coords)
  {
    float h = height[c.x + c.y * cx];

    if (!first) {
      float dh = 5*(h0 - h);
      //len += 0.001f + dh * dh;
      len += dh * dh;
    }

    first = false;
    h0 = h;
  }

  return len;
}

// ----------------------------------------------------------------------------
void findRoad(Path* pp, const x3d::vector2& pt0, const x3d::vector2& pt1, float* height, int cx, int cy, int level)
{
  const float dmin = 150;

  //float dp = 2 * (pt0 - pt1).length() / level;
  float dp = 100;
  x3d::vector2 pt = 0.5f * (pt0 + pt1);
  x3d::vector2 d = x3d::vector2(dp, dp);

  Path p0;
  p0.push_back(pt0);
  p0.push_back(pt);
  p0.push_back(pt1);
  float len0 = evaluate(p0, height, cx, cy);

  Path pmin = p0;
  float len = len0;
  bool cont = false;

  for (int i = 0; i < level*2000; i++)
  {
    float ofsx = 0.5 * d.u0() - randn(d.u0());
    float ofsy = 0.5 * d.u1() - randn(d.u1());

    x3d::vector2 pt(pt.u0() + ofsx, pt.u1() + ofsy);
    x3d::vector2 d0 = pt0 - pt;
    x3d::vector2 d1 = pt1 - pt;

    if  (pt.u0() < 0 || pt.u1() < 0
      || pt.u0() >= cx || pt.u1() >= cy
      || d0.length2() < 100 || d1.length2() < 100)
    {
      continue;
    }

    Path p;
    p.push_back(pt0);
    p.push_back(pt);
    p.push_back(pt1);

    float l = evaluate(p, height, cx, cy);
    if (l < len) {
      len = l;
      pmin = p;
      cont = true;
    }
  }

  printf("%d, ", (int)len);
  *pp = pmin;

  if (level > 1)
  {
    Path p;

    float d0 = (pmin[0] - pmin[1]).length();
    if (d0 > dmin) {
      Path p1;
      findRoad(&p1, pmin[0], pmin[1], height, cx, cy, level - 1);
      for (int i = 0; i < p1.size() - 1; i++) {
        p.push_back(p1[i]);
      }
    }
    else {
      p.push_back(pmin[0]);
    }

    float d1 = (pmin[1] - pmin[2]).length();
    if (d1 > dmin) {
      Path p2;
      findRoad(&p2, pmin[1], pmin[2], height, cx, cy, level - 1);
      for (int i = 0; i < p2.size(); i++) {
        p.push_back(p2[i]);
      }
    }
    else {
      p.push_back(pmin[1]);
      p.push_back(pmin[2]);
    }

    *pp = p;
  }
}

// ----------------------------------------------------------------------------
template <typename T>
void drawRoad(gfx::image_t& img, const Path& p, T color)
{
  const uint8_t col2 = 0x20;
  auto c0 = p[0];
  for (int i = 0; i+1 < p.size(); i+=1)
  {
    x3d::vector2 p0 = p[std::max(0, i - 1)];
    x3d::vector2 p1 = p[i];
    x3d::vector2 p2 = p[i+1];
    x3d::vector2 p3 = p[std::min<int>(p.size() - 1, i + 2)];

    x3d::vector2 bezier[4] = {
      p1,
      p1 + 0.1f * (p1 - p0),
      p2 + 0.1f * (p2 - p3),
      p2
    };
    for (int j = 0; j < 8; j++)
    {
      x3d::vector2 c = x3d::bezierPoint3(.125f * j, bezier);
      //x3d::vector2 c = p[i + 1];
      gfx::drawLine(img, (int)c0.u0(), (int)c0.u1(), (int)c.u0(), (int)c.u1(), color);

      gfx::drawLine(img, (int)c.u0()-2, (int)c.u1(), (int)c.u0()+3, (int)c.u1(), col2);
      gfx::drawLine(img, (int)c.u0(), (int)c.u1()-2, (int)c.u0(), (int)c.u1()+3, col2);
      c0 = c;
    }
  }
  
  x3d::vector2 c = p[p.size() - 1];
  gfx::drawLine(img, (int)c0.u0(), (int)c0.u1(), (int)c.u0(), (int)c.u1(), color);
}

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

  float* height = new float[cx * cy];

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
      
      height[x + y * cx] = d;

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

  x3d::vector2 p0((int)pts[0].pt.x, (int)pts[0].pt.y);
  x3d::vector2 p1((int)pts[1].pt.x, (int)pts[1].pt.y);
  x3d::vector2 p2((int)pts[2].pt.x, (int)pts[2].pt.y);
  x3d::vector2 p3((int)pts[3].pt.x, (int)pts[3].pt.y);

  Path r0;
  findRoad(&r0, p0, p1, height, cx, cy, 5);
  drawRoad(img, r0, color);

  Path r1;
  findRoad(&r1, p1, p3, height, cx, cy, 5);
  drawRoad(img, r1, color);

  Path r2;
  findRoad(&r2, p2, p3, height, cx, cy, 5);
  drawRoad(img, r2, color);

  Path r3;
  findRoad(&r3, p0, p2, height, cx, cy, 5);
  drawRoad(img, r3, color);

  gfx::saveBMPFile("g:\\Temp\\roads.bmp", geo, img, true);


  printf("done. map saved.\n");

  delete img.data[0];
  delete[] height;
}

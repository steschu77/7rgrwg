#include "main.h"
#include <list>

#include "gen.h"

#include "gfx/gfx.h"
#include "filter/Filters.h"
#include "world/BlockIds.h"

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
void gen::generateHeightMap(world::world_t* pWorld)
{
  filter::Perlin perlinSrc(1.0, 2.0, 0.5, 3);
  filter::ScaleBias perlin(&perlinSrc, 0.5, 0.5);
  filter::RidgedMulti ridged(1.0, 2.0, 3);
  filter::WorleyNoise worley(5000);
  filter::Perlin scatter(32.0, 2.1, 0.6, 4);

  unsigned cx = pWorld->cxMeters();
  unsigned cy = pWorld->cyMeters();

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

      pWorld->height[y][x] = d;
      pWorld->block[y][x] = id;

      uint8_t color = static_cast<uint8_t>(d);
      gfx::putPixel(img, x, y, color);
    }
  }

  gfx::saveBMPFile("g:\\Temp\\test.bmp", geo, img, true);
  printf("done. map saved.\n");

  delete img.data[0];
}

// ============================================================================
typedef int cenid;
typedef int corid;
typedef int edgeid;

// ============================================================================
struct poly_t
{
  poly_t();

  graph::center_t pt;

  bool water;
  bool ocean;
  bool coast;
  bool border;

  int biome;
  int elevation;
  float moisture;

  std::list<edgeid> edges;
  std::list<corid> verts;
};

// ============================================================================
struct vert_t
{
  graph::point_t pt;

  bool water;
  bool ocean;
  bool coast;
  bool border;

  float elevation;
  float moisture;

  std::list<cenid> touches;
  std::list<edgeid> protrudes;

  int river;
  corid downslope;
  corid watershed;
  int watershed_size;
};

// ============================================================================
struct edge_t
{
  edgeid id;
  cenid d0;
  cenid d1;
  corid v0;
  corid v1;
  graph::corner_t p0;
  graph::corner_t p1;

  int river;
  bool border;
};

// ============================================================================
typedef std::vector<poly_t> polys_t;
typedef std::vector<edge_t> edges_t;
typedef std::vector<vert_t> verts_t;

// ============================================================================
struct map_t
{
  polys_t polys;
  edges_t edges;
  verts_t verts;
};

// ============================================================================
poly_t::poly_t()
: water(false)
, ocean(false)
, coast(false)
, border(false)
, biome(0)
, elevation(0)
, moisture(0)
{
}

// ============================================================================
static double genNoise()
{
  return (double)rand() / 32767.0;
}


// ============================================================================
void createMap(const graph::segments_t& segs, const graph::centers_t& pts, map_t& m, const world::world_t& world)
{
  double cx = world.cxMeters();
  double cy = world.cyMeters();

  int cPolys = pts.size();
  int cEdges = segs.size();

  int idx = 0;

  int maxid = 0;
  for (graph::segments_t::const_iterator i = segs.begin(); i != segs.end(); i++) {
    maxid = std::max<int>(i->p0.id, maxid);
    maxid = std::max<int>(i->p1.id, maxid);
  }

  m.polys.resize(pts.size());
  m.edges.resize(segs.size());
  m.verts.resize(maxid + 1);

  for (int i = 0; i < cPolys; i++) {
    m.polys[i].pt = pts[i];
  }

  for (int i = 0; i < cEdges; i++)
  {
    const graph::segment_t& seg = segs[i];
    graph::corner_t p0 = seg.p0;
    graph::corner_t p1 = seg.p1;
    graph::center_t q0 = seg.c0;
    graph::center_t q1 = seg.c1;

    int a = 0;
    if (((a = graph::clipLine(p0.pt, p1.pt, cx, cy)) & 1) != 0)
    {
      bool border = (a & 6) != 0;

      m.verts[p0.id].pt = p0.pt;
      m.verts[p1.id].pt = p1.pt;

      m.verts[p0.id].border = (a & 2) != 0;
      m.verts[p1.id].border = (a & 4) != 0;

      m.verts[p0.id].protrudes.push_back(i);
      m.verts[p1.id].protrudes.push_back(i);

      m.verts[p0.id].touches.push_back(q0.id);
      m.verts[p0.id].touches.push_back(q1.id);
      m.verts[p1.id].touches.push_back(q0.id);
      m.verts[p1.id].touches.push_back(q1.id);

      m.edges[i].d0 = q0.id;
      m.edges[i].d1 = q1.id;
      m.edges[i].v0 = p0.id;
      m.edges[i].v1 = p1.id;
      m.edges[i].p0.pt = m.verts[p0.id].pt;
      m.edges[i].p1.pt = m.verts[p1.id].pt;
      m.edges[i].border = border;

      m.polys[q0.id].edges.push_back(i);
      m.polys[q1.id].edges.push_back(i);

      if (border) {
        m.polys[q0.id].border = true;
        m.polys[q1.id].border = true;
      }

      m.polys[q0.id].verts.push_back(p0.id);
      m.polys[q0.id].verts.push_back(p1.id);
      m.polys[q1.id].verts.push_back(p0.id);
      m.polys[q1.id].verts.push_back(p1.id);
    }
    idx++;
  }
}

// ============================================================================
gfx::point_t cvt2point(const graph::point_t& pt) {
  return gfx::point_t(pt.x, pt.y);
}

// ============================================================================
gfx::point_t cvt2point(const graph::center_t& ct) {
  return gfx::point_t(ct.pt.x, ct.pt.y);
}

// ============================================================================
struct Blend2 : public filter::Filter
{
  Blend2(filter::Filter* pf0, filter::Filter* pf1) : filter::Filter(pf0, pf1) {}

  double GetValue(double x, double y, double z) const
  {
    double v0 = _pSource[0]->GetValue(x, y, z);
    double v1 = _pSource[1]->GetValue(x, y, z);
  
    double a = (v1 - (112.0f/96.0f-1.0f));

    if (a > 0) {
      a = pow(a, .7) * 0.5 + 0.5;
    }
    else {
      a = -pow(-a, .7) * 0.5 + 0.5;
    }

    v0 = (v0 * .2f + (128.0f / 96.0f - 1.0f));

    return noise::LinearInterp(v1, v0, a);
  }
};

// ============================================================================
struct noise_map_t
{
  noise_map_t()
    : r(1.0, 2.0, 3, 0)
    , p(2.0, 2.1, 0.6, 2, 0)
    , b(&p, &r)
    , s(32.0, 2.1, 0.6, 4, 0) {}

  float height(int ix, int iy) const {
    double x = ix / 512.0;
    double y = iy / 512.0;
    return (b.GetValue(x, y, 0) + 1.0) * 96.0f;
  }

  float scatter(int ix, int iy) const {
    double x = ix / 512.0;
    double y = iy / 512.0;
    return s.GetValue(x, y, 0) * 8.0f;
  }

  float base(int ix, int iy) const {
    double x = ix / 512.0;
    double y = iy / 512.0;
    return p.GetValue(x, y, 0) * 4.0f;
  }

  filter::RidgedMulti r;
  filter::Perlin p;
  Blend2 b;
  filter::Perlin s;
};

// ============================================================================
void drawPolys(const noise_map_t& map, const map_t& m, gfx::image_t& img, const gfx::imagegeo_t& geo)
{
  unsigned cx = geo.cx;
  unsigned cy = geo.cy;

  gfx::canvas_t canvas(cy);
  gfx::clear<uint32_t>(img, geo, 0);

  for (unsigned iy = 0; iy < cy; ++iy) {
    for (unsigned ix = 0; ix < cx; ++ix)
    {
      float s = map.height(ix, iy);
      uint32_t color = static_cast<uint32_t>(s*256);

      gfx::putPixel(img, ix, iy, color);
    }
  }

  int k = 0;
  for (std::vector<poly_t>::const_iterator i = m.polys.begin(); i != m.polys.end(); i++, k++)
  {
    int x = i->pt.pt.x;
    int y = i->pt.pt.y;

    float s = map.height(x, y) + map.scatter(x, y);
    if (s < 96 || s > 128) {
      continue;
    }

    uint32_t color = static_cast<uint32_t>(s*256);
    uint32_t f[1] = { color };

    for (std::list<int>::const_iterator j = i->edges.begin(); j != i->edges.end(); j++)
    {
      const edge_t& e = m.edges[*j];

      gfx::point_t v0[3] = {
        cvt2point(i->pt),
        cvt2point(e.p0),
        cvt2point(e.p1)
      };

      gfx::drawPolyStrip(img, canvas, v0, f, 1);
    }
  }

#if 0
  int cEdges = m.edges.size();
  for (int i = 0; i < cEdges; i++) {
    const edge_t& e = m.edges[i];
    gfx::drawLine(img, (int)e.p0.x, (int)e.p0.y, (int)e.p1.x, (int)e.p1.y, 0xffffffff);
  }

  int cVerts = m.verts.size();
  for (int i = 0; i < cVerts; i++) {
    const vert_t& v = m.verts[i];
    gfx::putPixel(img, (int)v.pt.x, (int)v.pt.y, 0x000000);
  }
#endif

  gfx::saveBMPFile("g:\\Temp\\test2.bmp", geo, img, true);
}

// ============================================================================
void drawRivers(const noise_map_t& map, const map_t& m, gfx::image_t& deco, const gfx::imagegeo_t& geo)
{
  unsigned cx = geo.cx;
  unsigned cy = geo.cy;

  int n = 3 + randn(3);

  gfx::stride_t stride(gfx::gfxStride(geo.cf, cx));
  gfx::canvas_t canvas(cy);

  gfx::image_t img(stride, gfx::gfxPlaneSize(geo.cf, stride, cy));
  gfx::clear<uint32_t>(img, geo, 0);

  for (int i = 0; i < n; i++)
  {
    int idx = randn(m.verts.size());

    const vert_t& vStart = m.verts[idx];

    const graph::point_t& pt = vStart.pt;
    float h = map.height(pt.x, pt.y);

    if (h < 80) {
      continue;
    }

    std::vector<int> idcs;
    idcs.push_back(idx);

    bool minimize = true;

    while (minimize)
    {
      minimize = false;
      const vert_t& v = m.verts[idcs.back()];

      for (int j : v.protrudes)
      {
        auto check = [&map, &h, &idcs, &m](int i) {
          const vert_t& v = m.verts[i];
          float hv = map.height(v.pt.x, v.pt.y);
          if (hv < h) {
            h = hv;
            idcs.push_back(i);
            return true;
          }
          return false;
        };

        const edge_t& edge = m.edges[j];
        minimize = check(edge.v0) || check(edge.v1);
      }
    }

    std::vector<gfx::point_t> pts(idcs.size());
    std::vector<uint32_t> color(idcs.size(), 0xffffffff);

    for (int j = 0; j < idcs.size(); ++j) {
      pts[j] = cvt2point(m.verts[idcs[j]].pt);
    }
    gfx::drawLineStrip(img, &pts[0], &color[0], idcs.size()-1);
  }

  gfx::saveBMPFile("g:\\Temp\\test5.bmp", geo, img, true);
}

static const int X_NOISE_GEN = 1619;
static const int Y_NOISE_GEN = 31337;
static const int SEED_NOISE_GEN = 1013;
static const int SHIFT_NOISE_GEN = 8;

static double GradientNoise3D(double fx, double fy, int ix, int iy, int seed)
{
  int vectorIndex = (
      X_NOISE_GEN    * ix
    + Y_NOISE_GEN    * iy
    + SEED_NOISE_GEN * seed)
    & 0xffffffff;
  vectorIndex ^= (vectorIndex >> SHIFT_NOISE_GEN);
  vectorIndex &= 0xff;

  double xvGradient = noise::gRandomVectors[(vectorIndex << 2)];
  double yvGradient = noise::gRandomVectors[(vectorIndex << 2) + 1];

  double xvPoint = (fx - (double)ix);
  double yvPoint = (fy - (double)iy);

  return ((xvGradient * xvPoint) + (yvGradient * yvPoint)) * 2.12;
}

static double SCurve3(double a)
{
  return (a * a * (3.0 - 2.0 * a));
}

static double LinearInterp(double n0, double n1, double a)
{
  return ((1.0 - a) * n0) + (a * n1);
}

static double GradientCoherentNoise3D(double x, double y, int seed)
{
  int x0 = (int)x;
  int x1 = x0 + 1;
  int y0 = (int)y;
  int y1 = y0 + 1;

  double xs = SCurve3(x - (double)x0);
  double ys = SCurve3(y - (double)y0);

  double n0, n1, ix0, ix1, iy0, iy1;
  n0 = GradientNoise3D(x, y, x0, y0, seed);
  n1 = GradientNoise3D(x, y, x1, y0, seed);
  ix0 = LinearInterp(n0, n1, xs);
  n0 = GradientNoise3D(x, y, x0, y1, seed);
  n1 = GradientNoise3D(x, y, x1, y1, seed);
  ix1 = LinearInterp(n0, n1, xs);
  return LinearInterp(ix0, ix1, ys);
}

// ----------------------------------------------------------------------------
double perlin(double x, double y)
{
  double Value = 0.0;
  double curPersistence = 1.0;

  double Signal = GradientCoherentNoise3D(x, y, 0);
  Value = Signal * curPersistence;

  return Value;
}

// ============================================================================
void drawBaseMap(const gfx::imagegeo_t& geo)
{
  unsigned cx = geo.cx;
  unsigned cy = geo.cy;

  int n = 3 + randn(3);

  gfx::stride_t stride(gfx::gfxStride(geo.cf, cx));
  gfx::canvas_t canvas(cy);

  gfx::image_t img(stride, gfx::gfxPlaneSize(geo.cf, stride, cy));
  gfx::clear<uint32_t>(img, geo, 0);

  auto height = [](int ix, int iy) {
    double x = ix / 512.0;
    double y = iy / 512.0;
    return (perlin(x, y) + 1.5);
  };

  for (unsigned iy = 0; iy < cy; ++iy) {
    for (unsigned ix = 0; ix < cx; ++ix)
    {
      float s = height(ix, iy);
      float c = std::min(255.0f, std::max(0.0f, s * 96.0f));
      uint32_t color = static_cast<uint32_t>(c) + 0xff000000;

      gfx::putPixel(img, ix, iy, color);
    }
  }

  gfx::saveBMPFile("g:\\Temp\\test6.bmp", geo, img, true);
}

// ============================================================================
void drawStreetMap(gfx::image_t& img, const gfx::imagegeo_t& geo)
{
  unsigned cx = geo.cx;
  unsigned cy = geo.cy;

  gfx::canvas_t canvas(cy);

  gfx::clear<uint32_t>(img, geo, 0);

  graph::point_t ctrlPoints[] = {
    { 16, 128 },{ 64, 160 },
    { 128, 160 },{ 192, 160 },
    { 256, 160 },{ 320, 160 },
    { 384, 128 },{ 448,  128 },
    { 512 - 16,  64 }
  };

  gfx::point_t q0(0, 0), q1(0, 0);
  bool first = true;

  for (int i = 0; i < 4; i++)
  {
    graph::point_t p0 = ctrlPoints[2 * i];

    for (int j = 1; j <= 4; j++)
    {
      graph::point_t p1 = graph::bezierPoint(j / 4.0, ctrlPoints + 2 * i);
      graph::clipLine(p0, p1, cx, cy);

      graph::point_t d = graph::normalize(graph::perpendicular(p1 - p0), 6.0);

      uint32_t color[2] = { 0xffffffff, 0xffffffff };

      if (!first) {
        gfx::point_t gap[4] = {
          q0, cvt2point(p0 - d), q1, cvt2point(p0 + d)
        };

        gfx::drawPolyStrip(img, canvas, gap, color, 2);
      }

      gfx::point_t pts[4] = {
        cvt2point(p0 - d), cvt2point(p1 - d), cvt2point(p0 + d), cvt2point(p1 + d),
      };

      gfx::drawPolyStrip(img, canvas, pts, color, 2);

      uint32_t color2[2] = { 0xffff0000, 0xffffffff };
      gfx::drawLineStrip(img, pts, color2, 1);
      gfx::drawLineStrip(img, pts + 2, color2, 1);

      p0 = p1;

      q0 = cvt2point(p1 - d);
      q1 = cvt2point(p1 + d);
    }
  }
}

// ============================================================================
void generateDecorations(const gfx::image_t& heightMap, gfx::image_t& deco, const gfx::imagegeo_t& geo)
{
  unsigned cx = geo.cx;
  unsigned cy = geo.cy;

  gfx::canvas_t canvas(cy);

  for (int i = 0; i < 16; i++)
  {
    for (int i = 0; i < 80; i++)
    {
      int x = randn(cx);
      int y = randn(cy);
      bool isValid = gfx::getPixel<uint32_t>(deco, x, y) == 0x00000000;
      if (isValid) {
        gfx::putPixel<uint32_t>(deco, x, y, 0xfffff100 + i);
      }
    }
  }

  for (int i = 0; i < 64; i++)
  {
    int rx = randn(cx);
    int ry = randn(cy);

    float b = gfx::getPixel<uint32_t>(heightMap, rx, ry) / 256.0f;
    if (b > 96) {
      continue;
    }

    int n = 3 + randn(3);
    int cxArea = n + randn(6);
    int cyArea = n + randn(6);

    for (int j = 0; j < 10*n; j++)
    {
      int x = rx + randn(cxArea + 4);
      int y = ry + randn(cyArea + 4);

      if (x >= cx || y >= cy) {
        continue;
      }

      float b = gfx::getPixel<uint32_t>(heightMap, x, y) / 256.0f;
      if (b > 96) {
        continue;
      }

      bool isValid = gfx::getPixel<uint32_t>(deco, x, y) == 0x00000000;
      if (isValid) {
        gfx::putPixel<uint32_t>(deco, x, y, 0xfffff300);
      }
    }

    for (int j = 0; j < n; j++)
    {
      int x = rx + randn(cxArea);
      int y = ry + randn(cyArea);

      if (x >= cx || y >= cy) {
        continue;
      }

      float b = gfx::getPixel<uint32_t>(heightMap, x, y) / 256.0f;
      if (b > 96) {
        continue;
      }

      bool isValid = gfx::getPixel<uint32_t>(deco, x, y) == 0x00000000;
      if (isValid) {
        gfx::putPixel<uint32_t>(deco, x, y, 0xfffff200);
      }
    }
  }

  gfx::saveBMPFile("g:\\Temp\\test4.bmp", geo, deco, true);
}

// ============================================================================
void gen::generateSections(world::world_t* pWorld)
{
  double cx = pWorld->cxMeters();
  double cy = pWorld->cyMeters();

  gfx::ColorFormat cf = gfx::cfRGB8888;
  gfx::imagegeo_t geo(cf, cx, cy);
  gfx::stride_t stride(gfx::gfxStride(cf, cx));

  drawBaseMap(geo);

  size_t cPoints = 2500;
  graph::centers_t pts;
  for (int i = 0; i < cPoints; i++)
  {
    double x = genNoise() * cx;
    double y = genNoise() * cy;
    pts.push_back(graph::center_t(i, x, y));
  }

  graph::segments_t segs;
  voronoi(pts, segs);

  pts.clear();
  graph::easePoints(segs, pts, cPoints - 1, cx, cy);

  segs.clear();
  voronoi(pts, segs);

  pts.clear();
  graph::easePoints(segs, pts, cPoints - 1, cx, cy);

  segs.clear();
  voronoi(pts, segs);

  map_t map;
  createMap(segs, pts, map, *pWorld);

  noise_map_t noise;

  gfx::image_t heightMap(stride, gfx::gfxPlaneSize(cf, stride, cy));
  drawPolys(noise, map, heightMap, geo);

  drawRivers(noise, map, heightMap, geo);

  gfx::image_t blockMap(stride, gfx::gfxPlaneSize(cf, stride, cy));
  drawStreetMap(blockMap, geo);

  generateDecorations(heightMap, blockMap, geo);

  filter::Perlin scatter(32.0, 2.1, 0.6, 4);

  for (unsigned y = 0; y < cy; ++y) {
    printf("generate %d\r", y);
    for (unsigned x = 0; x < cx; ++x)
    {
      uint32_t id = gfx::getPixel<uint32_t>(blockMap, x, y);
      float b = gfx::getPixel<uint32_t>(heightMap, x, y) / 256.0f;
      float s = scatter.GetValue(x / 256.0, y / 256.0, 0) * 0.5f;

      bool isRoad = id == 0xffffffff;
      bool isBorder = id == 0xffff0000;
      bool isTree = (id & ~0xff) == 0xfffff100;
      bool isRock = (id & ~0xff) == 0xfffff200;
      bool isAzalea = (id & ~0xff) == 0xfffff300;

      float e = isRoad ? 80 : b;
      //float e = b;
      float d = std::min(std::max(e, 0.0f), 255.0f);

      pWorld->item[y][x] = idAir;
      pWorld->block[y][x] = idGrass;
      pWorld->height[y][x] = d;

      if (isRoad) {
        pWorld->block[y][x] = idAsphalt;
      }

      if (isBorder) {
        pWorld->block[y][x] = idGravel;
      }

      if (pWorld->block[y][x] == idGrass) {
        pWorld->item[y][x] = idTreeTallGrassDiagonal;
        pWorld->item[y][x] |= ((5 * rand()) / RAND_MAX) << 20;
      }
      if (isTree) {
        const uint32_t idTrees[16] = {
          idTreeBirch10m, idTreeBirch12m, idTreeBirch15m,
          idTreeMaple6m,  idTreeMaple13m, idTreeMaple15m, idTreeMaple16m, idTreeMountainPine16m, idTreeMountainPine19m,
          idTreeJuniper6m, idTreeMountainPine13m,
          idTreeMountainPine8m, idTreeMountainPine13m, idTreeMountainPine16m, idTreeMountainPine19m, idTreeMountainPine20m
          //idTreeWinterEverGreen, idTreeMaple17m
        };
        pWorld->item[y][x] = idTrees[id & 0xf];
      }
      if (isRock) {
        pWorld->item[y][x] = 0x00001280;
      }
      if (isAzalea) {
        pWorld->item[y][x] = idTreeAzalea;
      }
    }
  }
}

#include "main.h"
#include <list>

#include "gen.h"

#include "gfx/gfx.h"
#include "filter/Filters.h"
#include "world/BlockIds.h"

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
  graph::point_t p0;
  graph::point_t p1;

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
static void easePoints(const graph::segments_t& segs, graph::centers_t& pts, double cx, double cy)
{
  int sgs = 0;

  std::vector<edge_t> edges(segs.size());
  std::vector<poly_t> polys(2500);
  for (graph::segments_t::const_iterator i = segs.begin(); i != segs.end(); i++)
  {
    edge_t sg;
    sg.p0 = i->p0;
    sg.p1 = i->p1;

    if (graph::clipLine(sg.p0, sg.p1, cx, cy))
    {
      edges[sgs] = sg;
      polys[i->c0.id].edges.push_back(sgs);
      polys[i->c1.id].edges.push_back(sgs);
      sgs++;
    }
  }

  int k = 0;
  for (std::vector<poly_t>::iterator i = polys.begin(); i != polys.end(); i++, k++)
  {
    graph::center_t sum(k, 0, 0);
    double count = 0;
    for (std::list<int>::iterator j = i->edges.begin(); j != i->edges.end(); j++)
    {
      edge_t s = edges[*j];
      sum.x += s.p0.x;
      sum.y += s.p0.y;
      sum.x += s.p1.x;
      sum.y += s.p1.y;
      count += 2;
    }

    sum.x /= count;
    sum.y /= count;
    pts.push_back(sum);
  }
}

// ============================================================================
void createMap(const graph::segments_t& segs, const graph::centers_t& pts, map_t& m, const world::world_t& world)
{
  double cx = world.cxMeters();
  double cy = world.cyMeters();

  int cPolys = pts.size();
  int cEdges = segs.size();

  int idx = 0;
  int sgs = 0;

  int maxid = 0;
  for (graph::segments_t::const_iterator i = segs.begin(); i != segs.end(); i++) {
    maxid = std::max<int>(i->p0.id, maxid);
    maxid = std::max<int>(i->p1.id, maxid);
  }

  m.polys.resize(pts.size());
  m.edges.resize(segs.size());
  m.verts.resize(maxid);

  for (int i = 0; i < cPolys; i++) {
    m.polys[i].pt = pts[i];
  }

  for (int i = 0; i < cEdges; i++)
  {
    const graph::segment_t& seg = segs[i];
    graph::point_t p0 = seg.p0;
    graph::point_t p1 = seg.p1;
    graph::center_t q0 = seg.c0;
    graph::center_t q1 = seg.c1;

    int a = 0;
    if (((a = graph::clipLine(p0, p1, cx, cy)) & 1) != 0)
    {
      bool border = (a & 6) != 0;

      m.verts[p0.id].pt = p0;
      m.verts[p1.id].pt = p1;

      m.verts[p0.id].border = (a & 2) != 0;
      m.verts[p1.id].border = (a & 4) != 0;

      m.verts[p0.id].protrudes.push_back(sgs);
      m.verts[p1.id].protrudes.push_back(sgs);

      m.verts[p0.id].touches.push_back(q0.id);
      m.verts[p0.id].touches.push_back(q1.id);
      m.verts[p1.id].touches.push_back(q0.id);
      m.verts[p1.id].touches.push_back(q1.id);

      m.edges[i].d0 = q0.id;
      m.edges[i].d1 = q1.id;
      m.edges[i].v0 = p0.id;
      m.edges[i].v1 = p1.id;
      m.edges[i].p0 = m.verts[p0.id].pt;
      m.edges[i].p1 = m.verts[p1.id].pt;
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
  return gfx::point_t(ct.x, ct.y);
}

// ============================================================================
void drawPolys(const map_t& m, world::world_t* pWorld)
{
  unsigned cx = pWorld->cxMeters();
  unsigned cy = pWorld->cyMeters();

  gfx::ColorFormat cf = gfx::cfRGB8888;
  gfx::imagegeo_t geo(cf, cx, cy);
  gfx::canvas_t canvas(cy);
  gfx::stride_t stride(gfx::gfxStride(cf, cx));
  gfx::image_t img(stride, gfx::gfxPlaneSize(cf, stride, cy));

  gfx::clear<uint32_t>(img, geo, 0);

  filter::Perlin perlinSrc(1.0, 2.0, 0.5, 3);
  filter::ScaleBias perlin(&perlinSrc, 0.5, 0.5);
  filter::RidgedMulti ridged(1.0, 2.0, 3);
  filter::Perlin scatter(32.0, 2.1, 0.6, 4);

  int k = 0;
  for (std::vector<poly_t>::const_iterator i = m.polys.begin(); i != m.polys.end(); i++, k++)
  {
    double x = i->pt.x / 512.0;
    double y = i->pt.y / 512.0;

    float s = (ridged.GetValue(x, y, 0) + 1.0) * 64.0f + scatter.GetValue(x, y, 0) * 8.0f;
    uint32_t color = static_cast<uint32_t>(s);
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

  for (unsigned y = 0; y < cy; ++y) {
    printf("generate %d\r", y);
    for (unsigned x = 0; x < cx; ++x)
    {
      float s = scatter.GetValue(x/256.0, y/256.0, 0) * 0.5f;

      float b = static_cast<float>(gfx::getPixel<uint32_t>(img, x, y));
      float d = std::min(std::max(b + s, 0.0f), 255.0f);

      pWorld->height[y][x] = d;
      pWorld->block[y][x] = idStone;
    }
  }
}

// ============================================================================
void gen::generateSections(world::world_t* pWorld)
{
  double cx = pWorld->cxMeters();
  double cy = pWorld->cyMeters();

  graph::centers_t pts;
  for (int i = 0; i < 2500; i++)
  {
    double x = genNoise() * cx;
    double y = genNoise() * cy;
    pts.push_back(graph::center_t(i, x, y));
  }

  graph::segments_t segs;
  voronoi(pts, segs);

  pts.clear();
  easePoints(segs, pts, cx, cy);

  segs.clear();
  voronoi(pts, segs);

  pts.clear();
  easePoints(segs, pts, cx, cy);

  segs.clear();
  voronoi(pts, segs);

  map_t map;
  createMap(segs, pts, map, *pWorld);
  drawPolys(map, pWorld);
}

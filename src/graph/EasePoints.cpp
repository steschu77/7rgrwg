#include "main.h"
#include "graph/Graph.h"
#include <list>

typedef int edgeid;

// ============================================================================
struct ease_edge_t
{
  graph::corner_t p0;
  graph::corner_t p1;
};

// ============================================================================
struct ease_poly_t
{
  ease_poly_t() {}

  graph::center_t pt;
  std::list<edgeid> edges;
};

// ============================================================================
void graph::easePoints(const graph::segments_t& segs, graph::centers_t& pts, int maxid, double cx, double cy)
{
  int sgs = 0;

  std::vector<ease_edge_t> edges(segs.size());
  std::vector<ease_poly_t> polys(maxid + 1);
  for (graph::segments_t::const_iterator i = segs.begin(); i != segs.end(); i++)
  {
    ease_edge_t sg;
    sg.p0 = i->p0;
    sg.p1 = i->p1;

    if (graph::clipLine(sg.p0.pt, sg.p1.pt, cx, cy))
    {
      edges[sgs] = sg;
      polys[i->c0.id].edges.push_back(sgs);
      polys[i->c1.id].edges.push_back(sgs);
      sgs++;
    }
  }

  int k = 0;
  for (std::vector<ease_poly_t>::iterator i = polys.begin(); i != polys.end(); i++, k++)
  {
    graph::center_t sum(k, 0, 0);
    double count = 0;
    for (std::list<int>::iterator j = i->edges.begin(); j != i->edges.end(); j++)
    {
      ease_edge_t s = edges[*j];
      sum.pt.x += s.p0.pt.x;
      sum.pt.y += s.p0.pt.y;
      sum.pt.x += s.p1.pt.x;
      sum.pt.y += s.p1.pt.y;
      count += 2;
    }

    sum.pt.x /= count;
    sum.pt.y /= count;
    pts.push_back(sum);
  }
}

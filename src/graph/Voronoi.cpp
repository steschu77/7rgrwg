#include "main.h"
#include <queue>

#include "Graph.h"

// ============================================================================
class Voronoi
{
public:
  Voronoi(const graph::centers_t& pts);
  int process(graph::segments_t& segs);

protected:
  struct arc_t;
  struct seg_t;

  // ==========================================================================
  struct event_t
  {
    event_t(double xx, graph::point_t pp, arc_t *aa, const graph::center_t& c0, const graph::center_t& c1)
      : x(xx), p(pp), a(aa), valid(true), c0(c0), c1(c1) {}

    double x;
    graph::point_t p;
    arc_t *a;
    graph::center_t c0;
    graph::center_t c1;

    bool valid;
  };

  // ==========================================================================
  struct arc_t
  {
    arc_t(const graph::center_t& pp, arc_t* a=nullptr, arc_t* b=nullptr)
      : p(pp), prev(a), next(b), e(nullptr), s0(nullptr), s1(nullptr) {}

    graph::center_t p;
    arc_t *prev;
    arc_t *next;
    event_t *e;
    seg_t *s0, *s1;
  };

  // ==========================================================================
  struct seg_t
  {
    graph::point_t start, end;
    graph::center_t c0, c1;

    bool done;

    seg_t(const graph::point_t& p, const graph::center_t& c0, const graph::center_t& c1);

    // Set the end point_t and mark as "done."
    void finish(const graph::point_t& p) { if (done) return; end = p; done = true; }
  };

  // "Greater than" comparison, for reverse sorting in priority queue.
  struct center_gt {
    bool operator()(const graph::center_t& a, const graph::center_t& b) {return a.x==b.x ? a.y>b.y : a.x>b.x;}
  };

  struct event_gt {
    bool operator() (event_t* a, event_t* b) { return a->x > b->x; }
  };

  typedef std::priority_queue<graph::center_t,  std::vector<graph::center_t>, center_gt> sites_t; // site _events
  typedef std::priority_queue<event_t*, std::vector<event_t*>, event_gt> events_t; // circle _events

  sites_t _sites;
  events_t _events;

  // First item in the parabolic front linked list.
  arc_t* _root;

  int _idx_point;
  int _idx_seg;

  typedef std::vector<seg_t*> segs_t;
  segs_t _output;

  // Bounding box coordinates.
  double _X0;
  double _X1;
  double _Y0;
  double _Y1;

  void process_site();
  void process_event();
  void front_insert(graph::center_t p);
  void check_circle_event(arc_t *i, double x0);
  bool circle(const graph::center_t& a, const graph::center_t& b, const graph::center_t& c, double *x, graph::point_t *o);
  bool intersect(const graph::center_t& p, arc_t *i, graph::point_t *result);
  graph::point_t intersection(const graph::center_t& p0, const graph::center_t& p1, double l);
  void finish_edges();

private:
  Voronoi(const Voronoi&) = delete;
  Voronoi operator=(const Voronoi&) = delete;
};

// ============================================================================
Voronoi::Voronoi(const graph::centers_t& pts)
: _root(nullptr)
, _idx_point(0)
, _idx_seg(0)
, _X0(0)
, _X1(0)
, _Y0(0)
, _Y1(0)
{
  for (graph::centers_t::const_iterator i = pts.begin(); i != pts.end(); ++i)
  {
    const graph::center_t& p = *i;
    _sites.push(p);

    // Keep track of bounding box size.
    if (p.x < _X0) _X0 = p.x;
    if (p.y < _Y0) _Y0 = p.y;
    if (p.x > _X1) _X1 = p.x;
    if (p.y > _Y1) _Y1 = p.y;
  }

  // Add 20% margins to the bounding box.
  double dx = (_X1-_X0+1) / 5.0;
  double dy = (_Y1-_Y0+1) / 5.0;
  _X0 -= dx;
  _X1 += dx;
  _Y0 -= dy;
  _Y1 += dy;
}

// ----------------------------------------------------------------------------
int Voronoi::process(graph::segments_t& segs)
{
  // Process the queues; select the top element with smaller x coordinate.
  while (!_sites.empty())
  {
    if (!_events.empty() && _events.top()->x <= _sites.top().x)
      process_event();
    else
      process_site();
  }

  // After all points are processed, do the remaining circle _events.
  while (!_events.empty()) {
    process_event();
  }

  finish_edges(); // Clean up dangling edges.

  int idx = 0;
  for (segs_t::const_iterator i = _output.begin(); i != _output.end(); i++) {
    seg_t* p = *i;
    segs.push_back(graph::segment_t(idx++, p->start, p->end, p->c0, p->c1));
  }
  return 0;
}

// ----------------------------------------------------------------------------
void Voronoi::process_site()
{
  // Get the next point_t from the queue.
  graph::center_t p = _sites.top();
  _sites.pop();

  // Add a new arc_t to the parabolic front.
  front_insert(p);
}

// ----------------------------------------------------------------------------
void Voronoi::process_event()
{
  // Get the next event from the queue.
  event_t *e = _events.top();
  _events.pop();

  if (e->valid)
  {
    // Start a new edge.
    e->p.id = _idx_point++;
    seg_t *s = new seg_t(e->p, e->c0, e->c1);
    _output.push_back(s);

    // Remove the associated arc from the front.
    arc_t *a = e->a;
    if (a->prev) {
      a->prev->next = a->next;
      a->prev->s1 = s;
    }
    if (a->next) {
      a->next->prev = a->prev;
      a->next->s0 = s;
    }

    // Finish the edges before and after a.
    if (a->s0) a->s0->finish(e->p);
    if (a->s1) a->s1->finish(e->p);

    // Recheck circle events on either side of p:
    if (a->prev) check_circle_event(a->prev, e->x);
    if (a->next) check_circle_event(a->next, e->x);
  }
  delete e;
}

// ----------------------------------------------------------------------------
void Voronoi::front_insert(graph::center_t p)
{
  if (!_root) {
    _root = new arc_t(p);
    return;
  }

  // Find the current arc_t(s) at height p.y (if there are any).
  for (arc_t *i = _root; i; i = i->next)
  {
    graph::point_t z, zz;
    if (intersect(p,i,&z))
    {
      // New parabola intersects arc_t i.  If necessary, duplicate i.
      if (i->next && !intersect(p,i->next, &zz)) {
        i->next->prev = new arc_t(i->p, i, i->next);
        i->next = i->next->prev;
      }
      else i->next = new arc_t(i->p, i);
      i->next->s1 = i->s1;

      // Add p between i and i->next.
      i->next->prev = new arc_t(p, i, i->next);
      i->next = i->next->prev;

      i = i->next; // Now i points to the new arc_t.

      // Add new half-edges connected to i's endpoints.
      z.id = _idx_point++;
      seg_t* s0 = new seg_t(z, i->prev->p, i->p);
      seg_t* s1 = new seg_t(z, i->next->p, i->p);
      i->prev->s1 = i->s0 = s0;
      i->next->s0 = i->s1 = s1;
      _output.push_back(s0);
      _output.push_back(s1);

      // Check for new circle _events around the new arc_t:
      check_circle_event(i, p.x);
      check_circle_event(i->prev, p.x);
      check_circle_event(i->next, p.x);
      return;
    }
  }

  // Special case: If p never intersects an arc_t, append it to the list.
  arc_t *i;
  for (i = _root; i->next; i=i->next) ; // Find the last node.

  i->next = new arc_t(p, i);

  // Insert segment between p and i
  graph::point_t start;
  start.x = _X0;
  start.y = (i->next->p.y + i->p.y) / 2;
  start.id = _idx_point++;

  seg_t* s = new seg_t(start, p, i->p);
  i->s1 = i->next->s0 = s;
  _output.push_back(s);
}

// ----------------------------------------------------------------------------
// Look for a new circle event_t for arc_t i.
void Voronoi::check_circle_event(arc_t *i, double x0)
{
  // Invalidate any old event_t.
  if (i->e && i->e->x != x0)
    i->e->valid = false;
  i->e = NULL;

  if (!i->prev || !i->next)
    return;

  double x;
  graph::point_t o;

  if (circle(i->prev->p, i->p, i->next->p, &x, &o) && x > x0) {
    // Create new event.
    i->e = new event_t(x, o, i, i->prev->p, i->next->p);
    _events.push(i->e);
  }
}

// ----------------------------------------------------------------------------
// Find the rightmost point_t on the circle through a,b,c.
bool Voronoi::circle(const graph::center_t& a, const graph::center_t& b, const graph::center_t& c, double *x, graph::point_t *o)
{
  // Check that bc is a "right turn" from ab.
  if ((b.x-a.x)*(c.y-a.y) - (c.x-a.x)*(b.y-a.y) > 0) {
    return false;
  }

  // Algorithm from O'Rourke 2ed p. 189.
  double A = b.x - a.x;
  double B = b.y - a.y;
  double C = c.x - a.x;
  double D = c.y - a.y;
  double E = A*(a.x+b.x) + B*(a.y+b.y);
  double F = C*(a.x+c.x) + D*(a.y+c.y);
  double G = 2*(A*(c.y-b.y) - B*(c.x-b.x));

  if (G == 0) {
    return false;  // Points are co-linear.
  }

  // Point o is the center of the circle.
  o->x = (D*E-B*F)/G;
  o->y = (A*F-C*E)/G;

  // o.x plus radius equals max x coordinate.
  *x = o->x + sqrt( pow(a.x - o->x, 2) + pow(a.y - o->y, 2) );
  return true;
}

// ----------------------------------------------------------------------------
// Will a new parabola at point_t p intersect with arc_t i?
bool Voronoi::intersect(const graph::center_t& p, arc_t *i, graph::point_t *result)
{
  if (i->p.x == p.x) {
    return false;
  }

  double a = 0, b = 0;
  if (i->prev) // Get the intersection of i->prev, i.
    a = intersection(i->prev->p, i->p, p.x).y;
  if (i->next) // Get the intersection of i->next, i.
    b = intersection(i->p, i->next->p, p.x).y;

  if ((!i->prev || a <= p.y) && (!i->next || p.y <= b))
  {
    result->y = p.y;
    result->x = (i->p.x*i->p.x + (i->p.y-result->y)*(i->p.y-result->y) - p.x*p.x)
      / (2*i->p.x - 2*p.x);
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------------
// Where do two parabolas intersect?
graph::point_t Voronoi::intersection(const graph::center_t& p0, const graph::center_t& p1, double l)
{
  graph::point_t res;
  graph::center_t p = p0;

  double z0 = 2*(p0.x - l);
  double z1 = 2*(p1.x - l);

  if (p0.x == p1.x)
    res.y = (p0.y + p1.y) / 2;
  else if (p1.x == l)
    res.y = p1.y;
  else if (p0.x == l) {
    res.y = p0.y;
    p = p1;
  } else {
    // Use the quadratic formula.
    double a = 1/z0 - 1/z1;
    double b = -2*(p0.y/z0 - p1.y/z1);
    double c = (p0.y*p0.y + p0.x*p0.x - l*l)/z0
      - (p1.y*p1.y + p1.x*p1.x - l*l)/z1;

    res.y = ( -b - sqrt(b*b - 4*a*c) ) / (2*a);
  }
  // Plug back into one of the parabola equations.
  res.x = (p.x*p.x + (p.y-res.y)*(p.y-res.y) - l*l)/(2*p.x-2*l);
  return res;
}

// ----------------------------------------------------------------------------
void Voronoi::finish_edges()
{
  // Advance the sweep line so no parabolas can cross the bounding box.
  double l = _X1 + (_X1-_X0) + (_Y1-_Y0);

  // Extend each remaining segment to the new parabola intersections.
  for (arc_t *i = _root; i->next; i = i->next) {
    if (i->s1) {
      graph::point_t p = intersection(i->p, i->next->p, l*2);
      p.id = _idx_point++;
      i->s1->finish(p);
    }
  }
}

// ----------------------------------------------------------------------------
Voronoi::seg_t::seg_t(const graph::point_t& p, const graph::center_t& c0, const graph::center_t& c1)
: start(p), end(), c0(c0), c1(c1), done(false)
{
}

// ============================================================================
void graph::voronoi(const graph::centers_t& pts, graph::segments_t& segs)
{
  Voronoi v(pts);
  v.process(segs);
}

// -----------------------------------------------------------------------------
graph::center_t::center_t()
: id(0)
, x(0)
, y(0)
{
}

// -----------------------------------------------------------------------------
graph::center_t::center_t(int id, double x, double y)
: id(id)
, x(x)
, y(y)
{
}

// -----------------------------------------------------------------------------
graph::point_t::point_t()
: id(0)
, x(0)
, y(0)
{
}

// -----------------------------------------------------------------------------
graph::point_t::point_t(int id, double x, double y)
: id(id)
, x(x)
, y(y)
{
}

// -----------------------------------------------------------------------------
graph::segment_t::segment_t()
: id (0)
{
}

// -----------------------------------------------------------------------------
graph::segment_t::segment_t(int id, const point_t& p0, const point_t& p1, const center_t& c0, const center_t& c1)
: id(id)
, p0(p0)
, p1(p1)
, c0(c0)
, c1(c1)
{
}

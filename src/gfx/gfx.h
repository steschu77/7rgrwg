#pragma once

namespace gfx {

// ============================================================================
enum ColorFormat
{
  cfY8,
  cfRGB0888,
  cfRGB8888,
};

// ============================================================================
struct rgba_t
{
  uint8_t r, g, b, a;
};

// ============================================================================
struct point_t
{
  point_t(unsigned x, unsigned y);

  unsigned x, y;
};

// ============================================================================
union pixel_t
{
  pixel_t();
  pixel_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  pixel_t(uint32_t raw);

  uint32_t raw;
  rgba_t rgba;
};

// ============================================================================
struct stride_t
{
  explicit stride_t();
  explicit stride_t(int p);
  explicit stride_t(int p0, int p1, int p2);

  int p[3];
};

// ============================================================================
struct plane_size_t
{
  explicit plane_size_t();
  explicit plane_size_t(size_t p);
  explicit plane_size_t(size_t p0, size_t p1, size_t p2);

  size_t p[3];
};

// ============================================================================
struct imagegeo_t
{
  explicit imagegeo_t(ColorFormat cf, unsigned cx, unsigned cy);

  ColorFormat cf;
  unsigned cx;
  unsigned cy;
};

// ============================================================================
struct image_t
{
  explicit image_t();
  explicit image_t(const stride_t& s, const plane_size_t& psize);

  stride_t stride;
  uint8_t* data[3];
  uint32_t* palette;
};

// ============================================================================
struct canvas_t
{
  explicit canvas_t(unsigned cy);
  ~canvas_t();

  unsigned cy;
  unsigned yMin;
  unsigned yMax;

  int* edge[2];
};

stride_t gfxStride(ColorFormat cf, unsigned Pels, unsigned Alignment=1);
plane_size_t gfxPlaneSize(ColorFormat cf, const stride_t& Stride, unsigned Lines);
size_t gfxBufferSize(const plane_size_t& s);

template <typename T>
void putPixel(image_t& img, unsigned x, unsigned y, T color);

template <typename T>
T getPixel(image_t& img, unsigned x, unsigned y);

template <typename T>
void clear(image_t& img, const imagegeo_t& geo, T color);

void drawLine(image_t& img, int x0, int y0, int x1, int y1, uint32_t color);
void drawCircle(image_t& img, int x, int y, int r, uint32_t color);

void drawPolyStrip(image_t& img, canvas_t& canvas,
  const point_t* pVerts, const uint32_t* pColor, size_t cFaces);

void readBMPFileInfo(const char* FName, imagegeo_t* pInfo);
void readBMPFileData(const char* FName, image_t* pImage);
void saveBMPFile(const char* FName, const imagegeo_t& geo, const image_t& img, bool Overwrite);

} // namespace gfx


// ============================================================================
inline gfx::point_t::point_t(unsigned x, unsigned y)
: x(x), y(y)
{}

// ============================================================================
inline gfx::pixel_t::pixel_t()
: raw(0)
{
}

// ----------------------------------------------------------------------------
inline gfx::pixel_t::pixel_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  rgba.r = r;
  rgba.g = g;
  rgba.b = b;
  rgba.a = a;
}

// ----------------------------------------------------------------------------
inline gfx::pixel_t::pixel_t(uint32_t raw)
: raw(raw)
{
}

// ============================================================================
inline gfx::stride_t::stride_t()
{
  p[0] = 0;
  p[1] = 0;
  p[2] = 0;
}

// ----------------------------------------------------------------------------
inline gfx::stride_t::stride_t(int p0)
{
  p[0] = p0;
  p[1] = 0;
  p[2] = 0;
}

// ----------------------------------------------------------------------------
inline gfx::stride_t::stride_t(int p0, int p1, int p2)
{
  p[0] = p0;
  p[1] = p1;
  p[2] = p2;
}

// ============================================================================
inline gfx::plane_size_t::plane_size_t()
{
  p[0] = 0;
  p[1] = 0;
  p[2] = 0;
}

// ----------------------------------------------------------------------------
inline gfx::plane_size_t::plane_size_t(size_t p0)
{
  p[0] = p0;
  p[1] = 0;
  p[2] = 0;
}

// ----------------------------------------------------------------------------
inline gfx::plane_size_t::plane_size_t(size_t p0, size_t p1, size_t p2)
{
  p[0] = p0;
  p[1] = p1;
  p[2] = p2;
}

// ============================================================================
inline gfx::imagegeo_t::imagegeo_t(ColorFormat cf, unsigned cx, unsigned cy)
: cf(cf)
, cx(cx)
, cy(cy)
{}

// ============================================================================
inline gfx::image_t::image_t()
: palette(nullptr)
{
  data[0] = nullptr;
  data[1] = nullptr;
  data[2] = nullptr;
}

// ============================================================================
inline gfx::image_t::image_t(const stride_t& s, const plane_size_t& psize)
: stride(s)
, palette(nullptr)
{
  data[0] = new uint8_t[psize.p[0]];
  data[1] = new uint8_t[psize.p[1]];
  data[2] = new uint8_t[psize.p[2]];
}

// ============================================================================
inline gfx::canvas_t::canvas_t(unsigned cy)
: cy(cy)
, yMin(0)
, yMax(cy)
{
  edge[0] = new int[cy];
  edge[1] = new int[cy];
}

// ============================================================================
inline gfx::canvas_t::~canvas_t()
{
  delete[] edge[0];
  delete[] edge[1];
}

// ============================================================================
inline gfx::stride_t gfx::gfxStride(ColorFormat cf, unsigned Pels, unsigned Alignment)
{
  //static_assert((Alignment & (Alignment-1)) == 0, "alignment must be power of 2");
  unsigned Mask = ~(Alignment - 1);

  switch (cf)
  {
  case cfY8:
    return stride_t((Pels + Alignment - 1) & Mask, 0, 0);

  case cfRGB0888:
    return stride_t((Pels*3 + Alignment - 1) & Mask, 0, 0);

  case cfRGB8888:
    return stride_t((Pels*4 + Alignment - 1) & Mask, 0, 0);

  default:
    return stride_t();
  }
}

// ============================================================================
inline gfx::plane_size_t gfx::gfxPlaneSize(ColorFormat cf, const stride_t& stride, unsigned lines)
{
  plane_size_t factor = plane_size_t(1, 0, 0);

  plane_size_t psize;
  psize.p[0] = stride.p[0] * lines * factor.p[0];
  psize.p[1] = stride.p[1] * lines * factor.p[1];
  psize.p[2] = stride.p[2] * lines * factor.p[2];
  return psize;
}

// ============================================================================
template <typename T>
inline void gfx::putPixel(image_t& img, unsigned x, unsigned y, T color)
{
  size_t ofs = y * img.stride.p[0];
  reinterpret_cast<T*>(&img.data[0][ofs])[x] = color;
}

// ============================================================================
template <typename T>
inline T gfx::getPixel(image_t& img, unsigned x, unsigned y)
{
  size_t ofs = y * img.stride.p[0];
  return reinterpret_cast<T*>(&img.data[0][ofs])[x];
}

// ============================================================================
template <typename T>
inline void gfx::clear(image_t& img, const imagegeo_t& geo, T color)
{
  for (unsigned y = 0; y < geo.cy; ++y) {
    size_t ofs = y * img.stride.p[0];
    for (unsigned x = 0; x < geo.cx; ++x) {
      reinterpret_cast<T*>(&img.data[0][ofs])[x] = color;
    }
  }
}

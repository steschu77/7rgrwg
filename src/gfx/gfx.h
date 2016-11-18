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
  imagegeo_t(ColorFormat cf, unsigned cx, unsigned cy);

  ColorFormat cf;
  unsigned cx;
  unsigned cy;
};

// ============================================================================
struct image_t
{
  image_t();

  stride_t stride;
  uint8_t* data[3];
  uint32_t* palette;
};

void readBMPFileInfo(const char* FName, imagegeo_t* pInfo);
void readBMPFileData(const char* FName, image_t* pImage);
void saveBMPFile(const char* FName, const imagegeo_t& geo, const image_t& img, bool Overwrite);

} // namespace gfx

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

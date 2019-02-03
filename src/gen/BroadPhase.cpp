#include "main.h"
#include <list>

#include "filter/Filters.h"
#include "noise/NoiseGen.h"
#include "gfx/gfx.h"

#include "BroadPhase.h"

// ============================================================================
template <typename T>
T randn(const T& n)
{
  return (n * rand()) / RAND_MAX;
}

// ============================================================================
struct MapBorder : public filter::Filter
{
  double GetValue(double x, double y, double /*z*/) const
  {
    double x2 = 2.0 * (x - 0.5);
    double y2 = 2.0 * (y - 0.5);

    double d2 = x2 * x2 + y2 * y2;
    d2 = d2 * d2 * d2 * d2 * d2 * d2;

    double x0 = x;
    double x1 = 1 - x;
    double y0 = y;
    double y1 = 1 - y;

    double d0 = 2.1 * std::min(y0, y1);
    double d1 = 2.1 * std::min(x0, x1);

    d0 = 1.0 - std::min(1.0, d0 * d1);
    d0 = d0 * d0 * d0 * d0 * d0 * d0;
    d0 = d0 * d0;
    //d0 = 1.0 - d0;
    //d0 = 1.0 - d0 * d0 * d0 * d0 * d0;

    //return std::min(1.0, d0*d2);
    return std::min(1.0, d0);
  }
};

// ============================================================================
struct Blend2 : public filter::Filter
{
  Blend2(filter::Filter* pf0, filter::Filter* pf1) : filter::Filter(pf0, pf1) {}

  double GetValue(double x, double y, double z) const
  {
    double v0 = _pSource[0]->GetValue(x, y, z);
    double v1 = _pSource[1]->GetValue(x, y, z);
    double a  = _pSource[1]->GetValue(x, y, z);

    if (a > 0) {
      a = pow(a, .7) * 0.5 + 0.5;
    }
    else {
      a = -pow(-a, .7) * 0.5 + 0.5;
    }

    return noise::LinearInterp(v0, v1, a);
  }
};

class BoundRidgedMulti : public filter::Filter
{
public:
  BoundRidgedMulti(double f = 1.0, double l = 2.0, int o = 6, int s = 0)
    : _Frequency(f)
    , _Lacunarity(l)
    , _cOctaves(o)
    , _Seed(s)
  {
    double h = 1.0;
    double Frequency = 1.0;

    for (int i = 0; i < 30; i++) {
      // Compute weight for each frequency.
      _SpectralWeights[i] = pow(Frequency, -h);
      Frequency *= _Lacunarity;
    }
  }

  double boundNoise(double ox, double oy, double oz, double x, double y, double z, int octave, int seed) const
  {
    MapBorder border;
    double n = noise::GradientCoherentNoise3D(x, y, z, seed);
    double b = border.GetValue(ox, oy, oz);
    return (1-b)*n;
  }

  virtual double GetValue(double x, double y, double z) const
  {
    double ox = x;
    double oy = y;
    double oz = z;

    x *= _Frequency;
    y *= _Frequency;
    z *= _Frequency;

    double value = 0.0;
    double weight = 1.0;
    double offset = 1.0;
    double gain = 4.0;

    for (int curOctave = 0; curOctave < _cOctaves; curOctave++)
    {
      // Get the coherent-noise value.
      int Seed = (_Seed + curOctave) & 0x7fffffff;
      double signal = 0;

      signal = boundNoise(ox, oy, oz, x, y, z, curOctave, Seed);

      // Make the ridges.
      signal = fabs(signal);
      signal = offset - signal;

      // Square the signal to increase the sharpness of the ridges.
      signal *= signal;

      // The weighting from the previous octave is applied to the signal.
      // Larger values have higher weights, producing sharp points along the
      // ridges.
      signal *= weight;

      // Weight successive contributions by the previous signal.
      weight = std::min(std::max(signal * gain, 0.0), 1.0);

      // Add the signal to the output value.
      value += (signal * _SpectralWeights[curOctave]);

      // Prepare the next octave.
      x *= _Lacunarity;
      y *= _Lacunarity;
      z *= _Lacunarity;
    }

    return (value * 1.25) - 1.0;
  }

protected:
  double _Frequency;
  double _Lacunarity;

  int _cOctaves;
  int _Seed;

  /// Contains the spectral weights for each octave.
  double _SpectralWeights[30];
};


// ============================================================================
void gen::generateBiomes(world::world_t* pWorld)
{
  filter::RidgedMulti ridgedMapGen(2.0, 4.0, 3);
  filter::Abs absRiggedMapGen(&ridgedMapGen);
  BoundRidgedMulti boundRidgedMapGen(3.0, 4.0, 3);

  filter::Perlin perlinSrc(1.0, 2.0, 0.5, 3);
  filter::ScaleBias perlinMapGen(&perlinSrc, 0.5, 0.5);

  MapBorder borderMapGen;
  //filter::Multiply heightMapGen(&borderMapGen, &perlinMapGen);
  filter::Multiply heightMapGen(&borderMapGen, &absRiggedMapGen);

  unsigned cx = pWorld->cxMeters();
  unsigned cy = pWorld->cyMeters();
  double fcx = cx;
  double fcy = cy;

  gfx::imagegeo_t geo(gfx::cfY8, cx, cy);
  gfx::image_t img;
  img.stride.p[0] = cx;
  img.data[0] = new uint8_t [img.stride.p[0] * geo.cy];

  for (unsigned y = 0; y < cy; ++y) {
    printf("generate %d\r", y);
    for (unsigned x = 0; x < cx; ++x)
    {
      //float a = (heightMapGen.GetValue(x / fcx, y / fcy, 0) + 1.0) * 64.0f;
      float a = (boundRidgedMapGen.GetValue(x / fcx, y / fcy, 0) + 0.5) * 128.0f;
      //float a = (borderMapGen.GetValue(x / fcx, y / fcy, 0) + 1.0) * 64.0f;
      float d = std::min(std::max(a, 0.0f), 255.0f);

      uint8_t color = static_cast<uint8_t>(d);
      gfx::putPixel(img, x, y, color);
    }
  }

  gfx::saveBMPFile("g:\\Temp\\broad.bmp", geo, img, true);


  printf("done. map saved.\n");

  delete img.data[0];
}

#include "main.h"

#include "Billow.h"
#include "noise/NoiseGen.h"

// ----------------------------------------------------------------------------
filter::Billow::Billow(double f, double l, double p, int o, int s)
: _Frequency(f)
, _Lacunarity(l)
, _Persistence(p)
, _cOctaves(o)
, _Seed(s)
{
}

// ----------------------------------------------------------------------------
double filter::Billow::GetValue(double x, double y, double z) const
{
  double value = 0.0;
  double curPersistence = 1.0;

  x *= _Frequency;
  y *= _Frequency;
  z *= _Frequency;

  for (int curOctave = 0; curOctave < _cOctaves; curOctave++)
  {
    // Get the coherent-noise value from the input value and add it to the
    // final result.
    int seed = (_Seed + curOctave) & 0xffffffff;
    double signal = noise::GradientCoherentNoise3D(x, y, z, seed);
    signal = 2.0 * fabs(signal) - 1.0;
    value += signal * curPersistence;

    // Prepare the next octave.
    x *= _Lacunarity;
    y *= _Lacunarity;
    z *= _Lacunarity;
    curPersistence *= _Persistence;
  }

  value += 0.5;

  return value;
}

#include "main.h"

#include "Perlin.h"
#include "noise/NoiseGen.h"

// ----------------------------------------------------------------------------
filter::Perlin::Perlin(double f, double l, double p, int o, int s)
: _Frequency(f)
, _Lacunarity(l)
, _Persistence(p)
, _cOctaves(o)
, _Seed(s)
{
}

// ----------------------------------------------------------------------------
double filter::Perlin::GetValue (double x, double y, double z) const
{
  double Value = 0.0;
  double curPersistence = 1.0;

  x *= _Frequency;
  y *= _Frequency;
  z *= _Frequency;

  for (int curOctave = 0; curOctave < _cOctaves; curOctave++)
  {
    // Get the coherent-noise value from the input value and add it to the
    // final result.
    int Seed = (_Seed + curOctave) & 0xffffffff;
    double Signal = noise::GradientCoherentNoise3D (x, y, z, Seed);
    Value += Signal * curPersistence;

    // Prepare the next octave.
    x *= _Lacunarity;
    y *= _Lacunarity;
    z *= _Lacunarity;
    
    curPersistence *= _Persistence;
  }

  return Value;
}

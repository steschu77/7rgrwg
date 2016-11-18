#include "main.h"

#include "RidgedMulti.h"
#include "noise/NoiseGen.h"

filter::RidgedMulti::RidgedMulti(double f, double l, int o, int s)
: _Frequency(f)
, _Lacunarity(l)
, _cOctaves(o)
, _Seed(s)
{
  _calcSpectralWeights();
}

// Calculates the spectral weights for each octave.
void filter::RidgedMulti::_calcSpectralWeights()
{
  double h = 1.0;
  double Frequency = 1.0;
  
  for (int i = 0; i < 30; i++) {
    // Compute weight for each frequency.
    _SpectralWeights[i] = pow(Frequency, -h);
    Frequency *= _Lacunarity;
  }
}

// Multifractal code originally written by F. Kenton "Doc Mojo" Musgrave,
// 1998.  Modified by jas for use with libnoise.
double filter::RidgedMulti::GetValue(double x, double y, double z) const
{
  x *= _Frequency;
  y *= _Frequency;
  z *= _Frequency;

  double value  = 0.0;
  double weight = 1.0;
  double offset = 1.0;
  double gain = 2.0;

  for (int curOctave = 0; curOctave < _cOctaves; curOctave++)
  {
    // Get the coherent-noise value.
    int Seed = (_Seed + curOctave) & 0x7fffffff;
    double signal = noise::GradientCoherentNoise3D(x, y, z, Seed);

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

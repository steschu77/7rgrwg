#pragma once

#include "Filter.h"

namespace filter {

class RidgedMulti: public Filter
{
public:
  RidgedMulti(double f=1.0, double l=2.0, int o=6, int s=0);

  virtual double GetValue(double x, double y, double z) const;
  
protected:

  /// This method is called when the lacunarity changes.
  void _calcSpectralWeights();

  double _Frequency;
  double _Lacunarity;

  int _cOctaves;
  int _Seed;

  /// Contains the spectral weights for each octave.
  double _SpectralWeights[30];
};

}

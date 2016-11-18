#pragma once

namespace noise {

double GradientCoherentNoise3D(double x, double y, double z, int seed = 0);
double GradientNoise3D(double fx, double fy, double fz, int ix, int iy, int iz, int seed = 0);
int IntValueNoise3D(int x, int y, int z, int seed = 0);
double ValueCoherentNoise3D(double x, double y, double z, int seed = 0);
double ValueNoise3D(int x, int y, int z, int seed = 0);

} // noise

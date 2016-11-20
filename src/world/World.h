#pragma once

namespace world
{

struct world_t
{
  world_t(unsigned cx, unsigned cy);

  unsigned cxMeters() const {
    return cx * 16 * 32;
  }

  unsigned cyMeters() const {
    return cy * 16 * 32;
  }

  float* heightMap = nullptr;
  float** height = nullptr;

  uint32_t* blockMap = nullptr;
  uint32_t** block = nullptr;

  unsigned cx;
  unsigned cy;
};

}

inline world::world_t::world_t(unsigned cx, unsigned cy)
  : cx(cx)
  , cy(cy)
{
  unsigned cSquareMeters = cxMeters() * cyMeters();
  heightMap = new float[cSquareMeters];
  height = new float* [cyMeters()];

  std::fill_n(heightMap, cSquareMeters, 0.0f);

  for (unsigned y = 0; y < cyMeters(); ++y) {
    height[y] = &heightMap[cxMeters() * y];
  }

  blockMap = new uint32_t[cSquareMeters];
  block = new uint32_t*[cyMeters()];

  std::fill_n(blockMap, cSquareMeters, 0);

  for (unsigned y = 0; y < cyMeters(); ++y) {
    block[y] = &blockMap[cxMeters() * y];
  }
}

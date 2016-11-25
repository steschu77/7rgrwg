#pragma once

#include "graph/Graph.h"

namespace world
{

struct section_t
{
  graph::point_t center;
};

struct world_t
{
  world_t(unsigned cx, unsigned cy);

  unsigned cxMeters() const {
    return cx * 16 * 32;
  }

  unsigned cyMeters() const {
    return cy * 16 * 32;
  }

  // [0.0 .. 256.0f)
  float* heightMap = nullptr;
  float** height = nullptr;

  // [0.0 .. 256.0-height)
  float* waterMap = nullptr;
  float** water = nullptr;

  // BlockId
  uint32_t* blockMap = nullptr;
  uint32_t** block = nullptr;

  // BlockId + rotation
  uint32_t* itemMap = nullptr;
  uint32_t** item = nullptr;

  std::vector<section_t> sections;

  unsigned cx;
  unsigned cy;
};

}

template <typename T>
inline void allocT(T*& p, T**& pp, unsigned cx, unsigned cy)
{
  unsigned cSquare = cx * cy;

  p = new T[cSquare];
  std::fill_n(p, cSquare, static_cast<T>(0));

  pp = new T* [cy];
  for (unsigned y = 0; y < cy; ++y) {
    pp[y] = &p[cx * y];
  }
}

inline world::world_t::world_t(unsigned cx, unsigned cy)
  : cx(cx)
  , cy(cy)
{
  allocT(heightMap, height, cxMeters(), cyMeters());
  allocT(waterMap, water, cxMeters(), cyMeters());
  allocT(blockMap, block, cxMeters(), cyMeters());
  allocT(itemMap, item, cxMeters(), cyMeters());
}

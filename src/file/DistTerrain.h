#pragma once

#include "world/World.h"

struct terrain_t
{
  terrain_t(unsigned cx, unsigned cy) : cx(cx), cy(cy)
  {
    allocT(heightMap, height, cx, cy);
    allocT(blockMap, block, cx, cy);
  }

  ~terrain_t()
  {
  }

  terrain_t(const terrain_t&) = delete;
  terrain_t operator = (const terrain_t&) = delete;

  unsigned cx, cy;

  uint8_t* heightMap = nullptr;
  uint8_t** height = nullptr;

  uint8_t* blockMap = nullptr;
  uint8_t** block = nullptr;
};

namespace file {

void loadTerrain(terrain_t** ppTerrain, const std::string& strFolder);
void saveTerrain(const terrain_t* pTerrain, const std::string& strFolder);

void exportTerrain(const terrain_t* pTerrain, const std::string& strFile);
void exportTerrainBlocks(const terrain_t* pTerrain, const std::string& strFile);

}

#pragma once

#include "Chunk.h"

struct region_t
{
  region_t(int rx=0, int rz=0) : x(rx), z(rz)
  {
    for (int cz = 0; cz < 32; cz++) {
      for (int cx = 0; cx < 32; cx++) {
        chunk[cz][cx] = nullptr;
      }
    }
  }

  ~region_t()
  {
    for (int cz = 0; cz < 32; cz++) {
      for (int cx = 0; cx < 32; cx++) {
        delete chunk[cz][cx];
      }
    }
  }

  region_t(const region_t&) = delete;
  region_t operator = (const region_t&) = delete;

  int x, z;

  chunk_t *chunk[32][32];
};

namespace file {

void encodeRegion(region_t** ppRegion, int rx, int rz);
void encodeRegion(region_t** ppRegion, int rx, int rz, const world::world_t* pWorld);
void loadRegion(region_t** ppRegion, const std::string& strFolder, int rx, int rz);
void saveRegion(const region_t* pRegion, const std::string& strFolder);
void loadRegionChunk(chunk_t** ppChunk, const std::string& strFolder, int rx, int rz, int cx, int cz);

}

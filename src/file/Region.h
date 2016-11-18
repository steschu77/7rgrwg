#pragma once

#include "Chunk.h"

struct region_t
{
  region_t(int x=0, int z=0) : x(x), z(z) { 
    for (int z = 0; z < 32; z++) {
      for (int x = 0; x < 32; x++) {
        chunk[z][x] = nullptr;
      }
    }
  }

  ~region_t() {
    for (int z = 0; z < 32; z++) {
      for (int x = 0; x < 32; x++) {
        delete chunk[z][x];
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
void loadRegion(region_t** ppRegion, const std::string& strFolder, int rx, int rz);
void saveRegion(const region_t* pRegion, const std::string& strFolder);

}

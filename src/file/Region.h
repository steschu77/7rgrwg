#pragma once

#include "Chunk.h"

struct region_t
{
  region_t(int x=0, int z=0) : x(x), z(z) {}

  int x, z;

  chunk_t *chunk[32][32];
};

namespace file {

void loadRegion(region_t** ppRegion, const std::string& strFolder, int rx, int rz);
void saveRegion(void** pp7rg, size_t* pc7rg, const region_t* pRegion);

}

#include "../main.h"

#include "Region.h"
#include "FileSystem.h"

// ============================================================================
void file::loadRegion(region_t** ppRegion, const std::string& strFolder, int rx, int rz)
{
  std::stringstream s;
  s << strFolder << "r." << rx << "." << rz << ".7rg";

  std::string strFile = s.str();

  region_t* pRegion = new region_t();

  size_t c7rg = 0;
  uint8_t* p7rg = nullptr;
  loadFile(&p7rg, &c7rg, s.str().c_str());

  for (int z = 0; z < 32; z++) {
    for (int x = 0; x < 32; x++) {
      file::loadChunk(p7rg+4, x, z, rx, rz, &pRegion->chunk[z][x]);
    }
  }

  delete[] p7rg;

  *ppRegion = pRegion;
}

// ============================================================================
void file::saveRegion(void** pp7rg, size_t* pc7rg, const region_t* pRegion)
{
}

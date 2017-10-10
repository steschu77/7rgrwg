#include "../main.h"

#include "Region.h"
#include "FileSystem.h"

// ============================================================================
void file::encodeRegion(region_t** ppRegion, int rx, int rz)
{
  region_t* pRegion = new region_t(rx, rz);

  for (int z = 0; z < 32; z++) {
    for (int x = 0; x < 32; x++) {
      file::encodeChunk(&pRegion->chunk[z][x], x, z, rx, rz);
    }
  }

  *ppRegion = pRegion;
}

// ============================================================================
void file::encodeRegion(region_t** ppRegion, int rx, int rz, const world::world_t* pWorld)
{
  region_t* pRegion = new region_t(rx, rz);

  for (int z = 0; z < 32; z++) {
    for (int x = 0; x < 32; x++) {
      file::encodeChunk(&pRegion->chunk[z][x], x, z, rx, rz, pWorld);
    }
  }

  *ppRegion = pRegion;
}

// ============================================================================
void file::testRegion(const region_t* pRegion, int rx, int rz)
{
  for (int z = 0; z < 32; z++) {
    for (int x = 0; x < 32; x++) {
      if (pRegion->chunk[z][x] != nullptr) {
        file::testChunk(pRegion->chunk[z][x], x, z, rx, rz);
      }
    }
  }
}

// ============================================================================
void file::loadRegion(region_t** ppRegion, const std::string& strFolder, int rx, int rz)
{
  std::stringstream s;
  s << strFolder << "r." << rx << "." << rz << ".7rg";

  std::string strFile = s.str();

  region_t* pRegion = new region_t(rx, rz);

  size_t c7rg = 0;
  uint8_t* p7rg = nullptr;
  loadFile(&p7rg, &c7rg, strFile.c_str());

  for (int z = 0; z < 32; z++) {
    for (int x = 0; x < 32; x++) {
      file::loadChunk(p7rg+4, x, z, rx, rz, &pRegion->chunk[z][x]);
    }
  }

  delete[] p7rg;

  *ppRegion = pRegion;
}

// ============================================================================
void file::loadRegionChunk(chunk_t** ppChunk, const std::string& strFolder, int rx, int rz, int cx, int cz)
{
  std::stringstream s;
  s << strFolder << "r." << rx << "." << rz << ".7rg";

  std::string strFile = s.str();

  chunk_t* pChunk = new chunk_t(cx, cz);

  size_t c7rg = 0;
  uint8_t* p7rg = nullptr;
  loadFile(&p7rg, &c7rg, strFile.c_str());

  file::loadChunk(p7rg+4, cx, cz, rx, rz, &pChunk);

  delete[] p7rg;

  *ppChunk = pChunk;
}

// ============================================================================
static uint32_t _saveChunk(const region_t* pRegion, int x, int z, FILE* f)
{
  int rx = pRegion->x;
  int rz = pRegion->z;

  const uint8_t* pBuffer = nullptr;
  uint32_t cBuffer = 0;
  file::saveChunk(pRegion->chunk[z][x], x, z, rx, rz, &pBuffer, &cBuffer);

  if (f != nullptr) {
    fwrite(pBuffer, 1u, cBuffer, f);
  }

  return cBuffer;
}

// ============================================================================
void file::saveRegion(const region_t* pRegion, const std::string& strFolder)
{
  int rx = pRegion->x;
  int rz = pRegion->z;

  std::stringstream s;
  s << strFolder << "r." << rx << "." << rz << ".7rg";

  std::string str = s.str();

  std::string dir = str.substr(0, str.find_last_of("/\\") + 1);
  file::createPath(dir.c_str());

  const uint32_t initialOffset = 0x3000u;
  uint32_t index[initialOffset/4] = { 0 };
  uint32_t offset = initialOffset;

  for (int z = 0; z < 32; z++) {
    for (int x = 0; x < 32; x++) {
      uint32_t size = _saveChunk(pRegion, x, z, nullptr);
      if (size != 0) {
        index[32*z + x]  = offset / 0x1000u;
        index[32*z + x] |= ((size + 0xfffu) / 0x1000u) << 24;
        offset += size + 0x1000u;
      }
    }
  }

  FILE* f = fopen(str.c_str(), "wb");
  if (f == 0) {
    throw;
  }

  uint8_t head[4] ={ '7', 'r', 'g', 0 };
  fwrite(head, sizeof(uint8_t), 4, f);

  fwrite(index, sizeof(uint32_t), initialOffset/4, f);

  for (int z = 0; z < 32; z++) {
    for (int x = 0; x < 32; x++) {
      uint32_t size = _saveChunk(pRegion, x, z, f);
      if (size != 0) {
        // seems that 9 additional '00's are needed, which in turn needs to be
        // rounded up to 0x1000 '00's.
        uint8_t block[0x1000] = { 0 };
        fwrite(block, sizeof(uint8_t), 0x1000, f);
      }
    }
  }

  fseek(f, 0, SEEK_END);
  fclose(f);
}

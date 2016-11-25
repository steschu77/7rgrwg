#include "main.h"
#include "file/Region.h"
#include "gen/gen.h"

// ============================================================================
static const std::string strFolder = "g:\\Temp\\test08\\Region\\";
static const std::string strFolder2 = "g:\\Temp\\test08\\Region2\\";

// ============================================================================
void testChunkEncoding(const world::world_t* pWorld)
{
  const int rx = 0;
  const int rz = 0;
  const int x = 0;
  const int z = 0;

  chunk_t* pChunk = nullptr;
  file::encodeChunk(&pChunk, x, z, rx, rz, pWorld);
  file::decodeChunk(pChunk);

  delete pChunk;
}

// ============================================================================
void testRegionEncoding(const world::world_t* pWorld)
{
  for (int rz = 0; rz < pWorld->cy; rz++)
  {
    for (int rx = 0; rx < pWorld->cx; rx++)
    {
      region_t* pRegion;
      file::encodeRegion(&pRegion, rx, rz, pWorld);
      file::saveRegion(pRegion, strFolder2);

      delete pRegion;
    }
  }
}

// ============================================================================
void testRegionEncoding()
{
  for (int rz = 0; rz < 2; rz++)
  {
    for (int rx = 0; rx < 2; rx++)
    {
      region_t* pRegion;
      file::encodeRegion(&pRegion, rx, rz);
      file::saveRegion(pRegion, strFolder2);

      delete pRegion;
    }
  }
}

// ============================================================================
void testRegionParsing()
{
  const int rxStart = -3;
  const int rzStart =  1;
  const int rxEnd = -2;
  const int rzEnd =  2;

  for (int rz = rzStart; rz <= rzEnd; rz++)
  {
    for (int rx = rxStart; rx <= rxEnd; rx++)
    {
      region_t* pRegion;
      file::loadRegion(&pRegion, strFolder, rx, rz);
      file::saveRegion(pRegion, strFolder2);

      delete pRegion;
    }
  }
}

// ============================================================================
void analyzeRegionChunk(int rx, int rz, int cx, int cz, const world::world_t* pWorld)
{
  chunk_t* pChunk0 = nullptr;
  //file::loadRegionChunk(&pChunk0, strFolder, rx, rz, cx, cz);
  file::encodeChunk(&pChunk0, cx, cz, rx, rz);

  chunk_t* pChunk1 = nullptr;
  file::encodeChunk(&pChunk1, cx, cz, rx, rz, pWorld);

  file::compareChunks(pChunk0, pChunk1);
  file::decodeChunk(pChunk0);
  file::decodeChunk(pChunk1);

  delete pChunk0;
  delete pChunk1;
}

// ============================================================================
int main()
{
  world::world_t* pWorld = new world::world_t(1, 1);
  //gen::generateHeightMap(pWorld);
  gen::generateSections(pWorld);

  //analyzeRegionChunk(0, 0, 0, 0, pWorld);

  //testChunkEncoding(pWorld);
  testRegionEncoding(pWorld);
  //testRegionEncoding();
  return 0;
}

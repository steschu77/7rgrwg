#include "main.h"
#include "file/Region.h"
#include "gen/gen.h"

// ============================================================================
static const std::string strFolder = "g:\\Temp\\test08\\Region\\";
static const std::string strFolder2 = "g:\\Temp\\test08\\Region2\\";

// ============================================================================
void testChunkEncoding()
{
  const int rx = 0;
  const int rz = 0;
  const int x = 0;
  const int z = 0;

  chunk_t* pChunk = nullptr;
  file::encodeChunk(&pChunk, x, z, rx, rz);
  file::decodeChunk(pChunk);

  delete pChunk;
}

// ============================================================================
void testRegionEncoding()
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
int main()
{
  //testRegionEncoding();
  gen::generateHeightMap();
  return 0;
}

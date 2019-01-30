#include "main.h"
#include "file/Region.h"
#include "file/DistTerrain.h"
#include "gen/gen.h"
#include "gen/RoadGen.h"

// ============================================================================
static const std::string strFolder = "g:\\Temp\\test10\\Navezgane\\Region\\";
static const std::string strFolder2 = "g:\\Temp\\test10\\Navezgane2\\Region\\";

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
  const int rxStart = 1;
  const int rzStart = 1;
  const int rxEnd = 1;
  const int rzEnd = 1;

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
  //testRegionParsing();
  //return 0;

  world::world_t* pWorld = new world::world_t(1, 1);
  gen::generateRoadMap(pWorld);
  
  //gen::generateHeightMap(pWorld);
  //gen::generateSections(pWorld);
  //testRegionEncoding(pWorld);

  //testChunkEncoding(pWorld);
  //testRegionEncoding();

  /*
  const std::string strDistTerrainFile("G:\\Temp\\test08\\heightinfo.dtm");
  terrain_t* pTerrain = nullptr;
  file::loadTerrain(&pTerrain, strDistTerrainFile);

  const std::string strTerrainFile("G:\\Temp\\heightinfo.bmp");
  file::exportTerrain(pTerrain, strTerrainFile);

  const std::string strTerrainBlockFile("G:\\Temp\\heightinfo_block.bmp");
  file::exportTerrainBlocks(pTerrain, strTerrainBlockFile);

  delete pTerrain;*/
  return 0;
}

#include "../main.h"

#include "DistTerrain.h"
#include "FileSystem.h"
#include "gfx/gfx.h"

// ============================================================================
void file::loadTerrain(terrain_t** ppTerrain, const std::string& strFile)
{
  size_t cDTM = 0;
  uint8_t* pDTM = nullptr;
  loadFile(&pDTM, &cDTM, strFile.c_str());

  const uint8_t* ptr = pDTM;
  uint32_t cx = reinterpret_cast<const uint32_t*>(ptr)[0];
  uint32_t cy = reinterpret_cast<const uint32_t*>(ptr)[1];

  terrain_t* pTerrain = new terrain_t(cx, cy);

  for (unsigned y = 0; y < cy; ++y)
  {
    for (unsigned x = 0; x < cx; ++x)
    {
      pTerrain->height[y][x] = *ptr++;
      pTerrain->block[y][x] = *ptr++;
    }
  }

  delete[] pDTM;

  *ppTerrain = pTerrain;
}

// ============================================================================
void file::saveTerrain(const terrain_t* pTerrain, const std::string& strFolder)
{}

// ============================================================================
void file::exportTerrain(const terrain_t* pTerrain, const std::string& strFile)
{
  unsigned cx = pTerrain->cx;
  unsigned cy = pTerrain->cy;

  gfx::imagegeo_t geo(gfx::cfY8, cx, cy);
  gfx::image_t img;
  img.stride.p[0] = cx;
  img.data[0] = new uint8_t [cx * cy];

  for (unsigned y = 0; y < cy; ++y) {
    for (unsigned x = 0; x < cx; ++x) {
      gfx::putPixel(img, x, y, pTerrain->height[y][x]);
    }
  }

  gfx::saveBMPFile(strFile.c_str(), geo, img, true);
  delete img.data[0];
}

// ============================================================================
void file::exportTerrainBlocks(const terrain_t* pTerrain, const std::string& strFile)
{
  unsigned cx = pTerrain->cx;
  unsigned cy = pTerrain->cy;

  gfx::imagegeo_t geo(gfx::cfY8, cx, cy);
  gfx::image_t img;
  img.stride.p[0] = cx;
  img.data[0] = new uint8_t [cx * cy];

  for (unsigned y = 0; y < cy; ++y) {
    for (unsigned x = 0; x < cx; ++x) {
      gfx::putPixel(img, x, y, pTerrain->block[y][x]);
    }
  }

  gfx::saveBMPFile(strFile.c_str(), geo, img, true);
  delete img.data[0];
}

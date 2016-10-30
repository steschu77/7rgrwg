#include "main.h"
#include "file/Region.h"

const uint8_t Tail[13] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};

// ============================================================================
int main()
{
//*
  const std::string strFolder = "g:\\Temp\\test08\\Region\\";

  const char* fNames[4] ={
    "g:\\Temp\\test08\\Region\\r.-3.2.7rg",
    "g:\\Temp\\test08\\Region\\r.-3.1.7rg",
    "g:\\Temp\\test08\\Region\\r.-2.2.7rg",
    "g:\\Temp\\test08\\Region\\r.-2.1.7rg",
  };

  FILE* f = fopen(fNames[0], "rb");

  const size_t cBuffer = 10000000;
  uint8_t* buffer = new uint8_t[cBuffer];
  const size_t cFile = fread(buffer, 1, cBuffer, f);
  fclose(f);

  const int rxStart = -3;
  const int rzStart =  1;
  const int rxEnd = -2;
  const int rzEnd =  2;

  for (int rz = rzStart; rz < rzEnd; rz++)
  {
    for (int rx = rxStart; rx < rxEnd; rx++)
    {
      region_t* pRegion;
      file::loadRegion(&pRegion, strFolder, rx, rz);
    }
  }/*/


  const char* fName = "g:\\tmp\\test8\\-72\\65";
  FILE* f = fopen(fName, "rb");
  const size_t cBuffer = 10000000;
  uint8_t* buffer = new uint8_t[cBuffer];
  const size_t cFile = fread(buffer, 1, cBuffer, f);
  fclose(f);

  analyzeChunk(buffer, cFile);//*/
  return 0;
}

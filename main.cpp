#include "chunk.h"

const uint8_t Tail[13] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};

// ============================================================================
int main()
{
//*
  const char* fNames[4] = {
    "g:\\Temp\\test08\\Region\\r.-3.2.7rg",
    "g:\\Temp\\test08\\Region\\r.-3.1.7rg",
    "g:\\Temp\\test08\\Region\\r.-2.2.7rg",
    "g:\\Temp\\test08\\Region\\r.-2.1.7rg",
  };

  for (int i = 0; i < 4; i++)
  {
    FILE* f = fopen(fNames[i], "rb");

    const size_t cBuffer = 10000000;
    uint8_t* buffer = new uint8_t [cBuffer];
    const size_t cFile = fread(buffer, 1, cBuffer, f);
    fclose(f);

    for (int z = 0; z < 32; z++) {
      for (int x = 0; x < 32; x++) {
        chunk_t* pChunk;
        loadChunk(buffer+4, x, z, &pChunk);
        delete pChunk;
      }
    }
    delete[] buffer;
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

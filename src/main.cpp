#include <string>
#include <sstream>
#include "file/Chunk.h"
#include "file/FileSystem.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1310) /*Visual Studio: A few warning types are not desired here.*/
#pragma warning( disable : 4244 ) /*implicit conversions: not warned by gcc -Wall -Wextra and requires too much casts*/
#pragma warning( disable : 4996 ) /*VS does not like fopen, but fopen_s is not standard C so unusable here*/
#endif /*_MSC_VER */

const uint8_t Tail[13] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
};

// ============================================================================
int main()
{
//*
  const std::string strFolder = "g:\\Temp\\test08\\Region\\";

  const int rxStart = -3;
  const int rzStart =  1;
  const int rxEnd = -2;
  const int rzEnd =  2;

  for (int rz = rzStart; rz < rzEnd; rz++)
  {
    for (int rx = rxStart; rx < rxEnd; rx++)
    {
      std::stringstream s;
      s << strFolder << "r." << rx << "." << rz << ".7rg";

      size_t cBuffer = 0;
      void* pBuffer = nullptr;
      file::loadFile(&pBuffer, &cBuffer, s.str().c_str());

      const uint8_t* p7rg = reinterpret_cast<const uint8_t*>(pBuffer);

      for (int z = 0; z < 32; z++) {
        for (int x = 0; x < 32; x++) {
          chunk_t* pChunk = nullptr;
          file::loadChunk(p7rg+4, x, z, rx, rz, &pChunk);
          delete pChunk;
        }
      }

      free(pBuffer);
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

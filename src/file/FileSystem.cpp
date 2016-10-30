#include "../main.h"

#include "FileSystem.h"

#include <Windows.h>
#undef min
#undef max

// ============================================================================
void file::createPath(const char *Path)
{
  char SubDir[260];
  strncpy(SubDir, Path, 260);

  char *Work = SubDir;

  while (strlen(Work) > 1)
  {
    char* sFSlash = strchr(Work, '/');
    char* sBSlash = strchr(Work, '\\');
    if (sFSlash == nullptr && sBSlash == nullptr) {
      throw;
    }

    char* sSlash = (sFSlash == nullptr) ? sBSlash : ((sBSlash == nullptr) ? sFSlash : std::min(sFSlash, sBSlash));
    if (sSlash == Work || sSlash[-1] == ':')
    {
      Work = sSlash + 1;
      continue;
    }

    Work = sSlash + 1;
    sSlash[0] = 0;

    if (!CreateDirectory(SubDir, nullptr))
    {
      DWORD errorCode = GetLastError();
      if (errorCode != ERROR_ALREADY_EXISTS) {
        throw;
      }
    }

    sSlash[0] = '\\';
  }
}

// ============================================================================
void file::loadFile(uint8_t** out, size_t* outsize, const char* filename)
{
  FILE* f = fopen(filename, "rb");
  if (f == nullptr) throw;

  fseek(f, 0, SEEK_END);
  size_t cBuffer = ftell(f);
  if (cBuffer == LONG_MAX) throw; // >= 4 GiB

  uint8_t* buffer = new uint8_t[cBuffer];

  fseek(f, 0, SEEK_SET);
  const size_t cFile = fread(buffer, 1, cBuffer, f);

  fclose(f);

  *out = buffer;
  *outsize = cBuffer;
}

// ============================================================================
void file::saveFile(const void* buffer, size_t buffersize, const char* filename)
{
  FILE* f = fopen(filename, "wb" );
  if (f == 0) {
    throw;
  }

  fwrite(buffer, 1, buffersize, f);
  fclose(f);
}

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
void file::loadFile(void** out, size_t* outsize, const char* filename)
{
  FILE* file = fopen(filename, "rb");
  if (file == nullptr) throw;

  fseek(file, 0, SEEK_END);

  size_t size = ftell(file);
  if (size == LONG_MAX) throw; // >= 4 GiB

  *out = malloc(size);
  if (*out == nullptr && size > 0) throw;

  fread(out, 1, size, file);

  fclose(file);

  *outsize = size;
}

// ============================================================================
void file::saveFile(const void* buffer, size_t buffersize, const char* filename)
{
  FILE* file;
  file = fopen(filename, "wb" );
  if (file == 0) {
    throw;
  }

  fwrite((char*)buffer, 1, buffersize, file);
  fclose(file);
}

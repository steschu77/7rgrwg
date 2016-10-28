#include "file.h"

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstring> /*for size_t*/
#include <algorithm>
#include <climits>

#include <Windows.h>

#undef min
#undef max

#if defined(_MSC_VER) && (_MSC_VER >= 1310) /*Visual Studio: A few warning types are not desired here.*/
#pragma warning( disable : 4244 ) /*implicit conversions: not warned by gcc -Wall -Wextra and requires too much casts*/
#pragma warning( disable : 4996 ) /*VS does not like fopen, but fopen_s is not standard C so unusable here*/
#endif /*_MSC_VER */

// ============================================================================
void createPath(const char *Path)
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
void loadFile(void** out, size_t* outsize, const char* filename)
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
void saveFile(const void* buffer, size_t buffersize, const char* filename)
{
  FILE* file;
  file = fopen(filename, "wb" );
  if (file == 0) {
    throw;
  }

  fwrite((char*)buffer, 1, buffersize, file);
  fclose(file);
}

#pragma once

struct chunk_t
{
  int x, z;
  const void* pRaw;
  size_t cRaw;

  const void* pZipped;
  size_t cZipped;
};

namespace file {

void loadChunk(const void* p7rg, int x, int z, int rx, int rz, chunk_t** ppChunk);
void decodeChunk(chunk_t* pChunk, const unsigned char* out, size_t outsize);

}

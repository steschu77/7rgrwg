#pragma once

struct chunk_t
{
  chunk_t(int x=0, int z=0) : x(x), z(z) {}
  ~chunk_t() {
    delete[] pRaw;
    delete[] pZipped;
    delete[] p7rg;
    delete[] pDeflate;
  }

  chunk_t(const chunk_t&) = delete;
  chunk_t operator = (const chunk_t&) = delete;

  int x, z;

  const uint8_t* pRaw = nullptr;
  size_t cRaw = 0;
  size_t cRaw4k = 0;

  const uint8_t* pZipped = nullptr;
  size_t cZipped = 0;

  uint8_t* p7rg = nullptr;
  size_t c7rg = 0;

  const uint8_t* pDeflate = nullptr;
  size_t cDeflate = 0;
  uint32_t crc32Deflate = 0;
};

namespace file {

void loadChunk(const void* p7rg, int x, int z, int rx, int rz, chunk_t** ppChunk);
void saveChunk(const chunk_t* pChunk, int x, int z, int rx, int rz, uint8_t const ** ppBuffer, size_t* pcBuffer);
void decodeChunk(chunk_t* pChunk);
void encodeChunk(chunk_t** ppChunk, int x, int z, int rx, int rz);

}

#include "../main.h"

#include "Chunk.h"
#include "ZipFile.h"
#include "Deflate.h"

// ============================================================================
void file::loadChunk(const void* buffer, int x, int z, int rx, int rz, chunk_t** ppChunk)
{
  int tx = (rx < 0) ? ((x + 1) & 31) : (x);
  int tz = (rz < 0) ? ((z + 1) & 31) : (z);

  int cx = 32 * rx + tx;
  int cz = 32 * rz + tz;

  int idx = x + 32 * z;

  const uint32_t ofs = reinterpret_cast<const uint32_t*>(buffer)[idx];

  if (ofs == 0) {
    return;
  }

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buffer);
  ptr += 0x1000 * (ofs & 0xffffff);
  
  //const uint32_t size = * (reinterpret_cast<const uint32_t*>(ptr));
  ptr += sizeof(uint32_t);

  // 00 74 74 63 00 xx xx xx xx, x = version (Alpha 15: 0x21, Alpha 16: 0x22)
  ptr += 9;

  const ZipLocalFileHeader* pLocalHead = reinterpret_cast<const ZipLocalFileHeader*>(ptr);
  ptr += sizeof(ZipLocalFileHeader);

  char ChunkName[260] = { 0 };
  memcpy(ChunkName, ptr, pLocalHead->fileNameLength);

  // filename: "xx/yy"
  ptr += pLocalHead->fileNameLength;
  ptr += pLocalHead->extraFieldLength;

  const unsigned char* in = reinterpret_cast<const unsigned char*>(ptr);
  unsigned char* out = new unsigned char [pLocalHead->sizeUncompressed];
  ptr += pLocalHead->sizeCompressed;

  size_t insize = pLocalHead->sizeCompressed;
  size_t outsize = pLocalHead->sizeUncompressed;

  file::inflate(&out, &outsize, in, insize);

  int cx2 = reinterpret_cast<const int*>(out)[0];
  int cz2 = reinterpret_cast<const int*>(out)[2];

  if (cx != cx2 || cz != cz2) throw;

  const ZipFileHeader* pFileHead = reinterpret_cast<const ZipFileHeader*>(ptr);
  ptr += sizeof(ZipFileHeader);
  ptr += pFileHead->fileNameLength;
  ptr += pFileHead->extraFieldLength;

  const ZipCentralDirEnd* pDirEnd = reinterpret_cast<const ZipCentralDirEnd*>(ptr);
  ptr += sizeof(ZipCentralDirEnd);
  ptr += pDirEnd->zipFileCommentLength;

  chunk_t* pChunk = new chunk_t;
  pChunk->pRaw = in;
  pChunk->cRaw = insize;
  pChunk->pZipped = out;
  pChunk->cZipped = outsize;

  *ppChunk = pChunk;
}

// ============================================================================
void file::decodeChunk(chunk_t* pChunk, const unsigned char* out, size_t outsize)
{
  const unsigned char* p0 = out;
  out += 0x14;

  const int sections = 64;
  for (int i = 0; i < sections; i++)
  {
    printf("b: %4d %06x  %02x", i, (int)(out-p0), out[0]);
    if (out[0] == 0) {
      out += 1;
      printf("\n");
      continue;
    }
    printf(" %02x", out[0]);
    if (out[1] == 0) {
      out += 2;
      out += 1;
    } else {
      out += 2;
      out += 16 * 16 * 4;
    }
    printf(" %02x\n", out[0]);
    if (out[0] == 0) {
      out += 1;
    } else {
      out += 1;
      out += 16 * 16 * 4 * 3;
    }
  }

  // 0f: solid, 00 air
  for (int i = 0; i < sections; i++)
  {
    printf("x: %4d %06x  %02x\n", i, (int)(out-p0), out[0]);
    if (out[0] == 0) {
      out += 1;
      out += 16 * 16 * 4;
    } else {
      out += 1;
      out += 1;
    }
  }

  // Height Map
  out += 16 * 16;

  // Another Map
  out += 16 * 16;

  // All 03's Map (biome?, 03 = pine forest)
  out += 16 * 16;

  // 03 00 00 00 0F 00 Map
  // 00 00 00 00 0F 00 Map
  out += 16 * 16 * 6;

  // +/- something Map
  out += 4;
  out += 16 * 16;

  // 7d/7E Map 
  out += 16 * 16;

  // +/- something Map
  out += 16 * 16;

  // fill level? -128 .. 127 (solid .. air)
  for (int i = 0; i < sections; i++)
  {
    printf("y: %4d %06x  %02x\n", i, (int)(out-p0), out[0]);
    if (out[0] == 0) {
      out += 1;
      out += 16 * 16 * 4;
    }
    else {
      out += 1;
      out += 1;
    }
  }

  // 00: solid, 0f: air
  for (int i = 0; i < sections; i++)
  {
    printf("z: %4d %06x  %02x\n", i, (int)(out-p0), out[0]);
    if (out[0] == 0) {
      out += 1;
      out += 16 * 16 * 4;
    }
    else {
      out += 1;
      out += 1;
    }
  }

  // all 0
  for (int i = 0; i < sections; i++)
  {
    printf("i: %4d %06x  %02x\n", i, (int)(out-p0), out[0]);
    if (out[0] == 0) {
      out += 1;
      out += 16 * 16 * 4;
    }
    else {
      out += 1;
      out += 1;
    }
  }

  // block damage
  for (int i = 0; i < sections; i++)
  {
    printf("j: %4d %06x  %02x\n", i, (int)(out-p0), out[0]);
    if (out[0] == 0) {
      out += 1;
      out += 16 * 16 * 4;
    }
    else {
      out += 1;
      out += 1;
    }
  }

  char fName[260];
  sprintf(fName, "g:\\tmp\\test8\\-72x65_1");

  FILE* f = fopen(fName, "wb");
  fwrite(out, 1, outsize - (out-p0), f);
  fclose(f);
}

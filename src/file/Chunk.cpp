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

  const uint32_t tmpOfsSize = reinterpret_cast<const uint32_t*>(buffer)[idx];
  if (tmpOfsSize == 0) {
    return;
  }

  const uint32_t ofs = 0x1000 * (tmpOfsSize & 0xffffff);
  const uint32_t size4k = 0x1000 * (tmpOfsSize >> 24);

  uint8_t* ptrChunk = new uint8_t[size4k];
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buffer);

  memcpy(ptrChunk, ptr + ofs, size4k);

  ptr = ptrChunk;

  const uint32_t size = * (reinterpret_cast<const uint32_t*>(ptr));
  ptr += sizeof(uint32_t);

  // 00 74 74 63 00 xx xx xx xx, x = version (Alpha 14: 0x21, Alpha 15: 0x22)
  ptr += 9;

  const ZipLocalFileHeader* pLocalHead = reinterpret_cast<const ZipLocalFileHeader*>(ptr);
  ptr += sizeof(ZipLocalFileHeader);

  char ChunkName[260] = { 0 };
  memcpy(ChunkName, ptr, pLocalHead->fileNameLength);

  // filename: "xx/zz"
  ptr += pLocalHead->fileNameLength;
  ptr += pLocalHead->extraFieldLength;

  size_t insize = pLocalHead->sizeCompressed;
  size_t outsize = pLocalHead->sizeUncompressed;

  const unsigned char* in = ptr;
  unsigned char* out = new unsigned char [outsize];

  ptr += pLocalHead->sizeCompressed;

  file::inflate(&out, &outsize, in, insize);

  int cx2 = reinterpret_cast<const int*>(out)[0];
  int cz2 = reinterpret_cast<const int*>(out)[2];

  if (cx != cx2 || cz != cz2) throw;

  const ZipFileHeader* pFileHead = reinterpret_cast<const ZipFileHeader*>(ptr);
  ptr += sizeof(ZipFileHeader);
  ptr += pFileHead->fileNameLength;
  ptr += pFileHead->extraFieldLength;
  ptr += pFileHead->fileCommentLength;

  const ZipCentralDirEnd* pDirEnd = reinterpret_cast<const ZipCentralDirEnd*>(ptr);
  ptr += sizeof(ZipCentralDirEnd);
  ptr += pDirEnd->zipFileCommentLength;

  chunk_t* pChunk = new chunk_t(x, z);
  pChunk->pRaw = ptrChunk;
  pChunk->cRaw = size;
  pChunk->cRaw4k = size4k;
  pChunk->pZipped = out;
  pChunk->cZipped = outsize;
  pChunk->c7rg = 16 * 16 * 256 * 8;
  pChunk->p7rg = new uint8_t [pChunk->c7rg];
  pChunk->pDeflate = in;
  pChunk->cDeflate = insize;
  pChunk->crc32Deflate = pFileHead->crc32;

  memset(pChunk->p7rg, 0, pChunk->c7rg);

  *ppChunk = pChunk;
}

// ============================================================================
void file::saveChunk(const chunk_t* pChunk, int x, int z, int rx, int rz, uint8_t const ** ppBuffer, size_t* pcBuffer)
{
  if (pChunk == nullptr) {
    return;
  }

  //int x = pChunk->x;
  //int z = pChunk->z;

  const int* pConstCoords = reinterpret_cast<const int*>(pChunk->pZipped);
  int* pCoords = const_cast<int*>(pConstCoords);

  //int x0 = pCoords[0];
  //int z0 = pCoords[2];
  
  int tx = (rx < 0) ? ((x + 1) & 31) : (x);
  int tz = (rz < 0) ? ((z + 1) & 31) : (z);

  int cx = 32 * rx + tx;
  int cz = 32 * rz + tz;

  pCoords[0] = cx;
  pCoords[2] = cz;

  uint8_t* p7rg = pChunk->p7rg;
  uint8_t* ptr = p7rg;

  ptr += sizeof(uint32_t);

  uint8_t head[5] = { 0x00, 0x74, 0x74, 0x63, 0x00 };
  memcpy(ptr, head, 5);
  ptr += 5;

  uint32_t version = 0x22; // Alpha 15
  *(reinterpret_cast<uint32_t*>(ptr)) = version;
  ptr += sizeof(uint32_t);

  char fileName[32] ={0};
  int fileNameLength = sprintf(fileName, "%d/%d", cx, cz);

  uint32_t crc32 = file::crc32(pChunk->pZipped, pChunk->cZipped);

  ZipLocalFileHeader* pLocalHead = reinterpret_cast<ZipLocalFileHeader*>(ptr);
  ptr += sizeof(ZipLocalFileHeader);
  pLocalHead->signature = zipsigLocalFileHeader;
  pLocalHead->zVersionNeeded.hostSystem = 20;
  pLocalHead->zVersionNeeded.zipVersion = 0;
  pLocalHead->flags = 0;
  pLocalHead->compressionMethod = zipcompDeflated;
  pLocalHead->fileTime = 29546;
  pLocalHead->fileDate = 18760;
  pLocalHead->crc32 = crc32;
  pLocalHead->sizeCompressed = pChunk->cDeflate;
  pLocalHead->sizeUncompressed = pChunk->cZipped;
  pLocalHead->fileNameLength = fileNameLength;
  pLocalHead->extraFieldLength = 0;

  memcpy(ptr, fileName, fileNameLength);
  ptr += pLocalHead->fileNameLength;

  ptr += pLocalHead->extraFieldLength;

  size_t outsize = 0;
  unsigned char* out = nullptr;

  const unsigned char* in = pChunk->pZipped;
  size_t insize = pChunk->cZipped;

  file::deflate(&out, &outsize, in, insize);
  pLocalHead->sizeCompressed = outsize;

  memcpy(ptr, out, outsize);
  free(out);

  //memcpy(ptr, pChunk->pDeflate, pChunk->cDeflate);
  ptr += pLocalHead->sizeCompressed;

  size_t cFileHead = 0;
  size_t ofsFileHead = ptr - (p7rg + 5 + 8);

  ZipFileHeader* pFileHead = reinterpret_cast<ZipFileHeader*>(ptr);
  pFileHead->signature = zipsigFileHeader;
  pFileHead->zVersionMadeBy.hostSystem = 20;
  pFileHead->zVersionMadeBy.zipVersion = 0;
  pFileHead->zVersionNeeded.hostSystem = 20;
  pFileHead->zVersionNeeded.zipVersion = 0;
  pFileHead->flags = 0;
  pFileHead->compressionMethod = zipcompDeflated;
  pFileHead->fileTime = 29546;
  pFileHead->fileDate = 18760;
  pFileHead->crc32 = crc32;
  pFileHead->sizeCompressed = pChunk->cDeflate;
  pFileHead->sizeUncompressed = pChunk->cZipped;
  pFileHead->fileNameLength = fileNameLength;
  pFileHead->extraFieldLength = 0;
  pFileHead->fileCommentLength = 0;
  pFileHead->startDisk = 0;
  pFileHead->fileAttribInternal = 0;
  pFileHead->fileAttribExternal = 0;
  pFileHead->ofsLocalHeader = 0;

  ptr += sizeof(ZipFileHeader);
  cFileHead += sizeof(ZipFileHeader);

  memcpy(ptr, fileName, fileNameLength);
  ptr += pFileHead->fileNameLength;
  cFileHead += pFileHead->fileNameLength;

  ptr += pFileHead->extraFieldLength;
  cFileHead += pFileHead->extraFieldLength;

  ptr += pFileHead->fileCommentLength;
  cFileHead += pFileHead->fileCommentLength;

  ZipCentralDirEnd* pDirEnd = reinterpret_cast<ZipCentralDirEnd*>(ptr);

  pDirEnd->signature = zipsigCentralDirEnd;
  pDirEnd->thisDiscNo = 0;
  pDirEnd->discStart = 0;
  pDirEnd->thisCentralDirEntries = 1;
  pDirEnd->centralDirEntries = 1;
  pDirEnd->centralDirSize = cFileHead;
  pDirEnd->centralDirStart = ofsFileHead;
  pDirEnd->zipFileCommentLength = 0;

  ptr += sizeof(ZipCentralDirEnd);
  ptr += pDirEnd->zipFileCommentLength;

  uint32_t c7rg = ptr - p7rg - 5;
  uint32_t c7rg4k = (c7rg + 0xfff) & ~0xfff;

  ptr = p7rg;
  *(reinterpret_cast<uint32_t*>(ptr)) = c7rg;

  *ppBuffer = p7rg;
  *pcBuffer = c7rg4k;

  /*/
  *ppBuffer = pChunk->pRaw;
  *pcBuffer = pChunk->cRaw4k;
  //*/
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

  // Another Height Map
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
}

#include "../main.h"

#include "Chunk.h"
#include "ZipFile.h"
#include "Deflate.h"

// ============================================================================
template <typename T>
T size_cast(size_t c) {
  return static_cast<T>(c);
}

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

  const int* pConstCoords = reinterpret_cast<const int*>(pChunk->pZipped);
  int* pCoords = const_cast<int*>(pConstCoords);

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

  size_t outsize = 0;
  unsigned char* out = nullptr;

  file::deflate(&out, &outsize, pChunk->pZipped, pChunk->cZipped);
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
  pLocalHead->sizeCompressed = size_cast<uint32_t>(outsize);
  pLocalHead->sizeUncompressed = size_cast<uint32_t>(pChunk->cZipped);
  pLocalHead->fileNameLength = fileNameLength;
  pLocalHead->extraFieldLength = 0;

  memcpy(ptr, fileName, fileNameLength);
  ptr += pLocalHead->fileNameLength;
  ptr += pLocalHead->extraFieldLength;

  memcpy(ptr, out, outsize);
  ptr += outsize;

  free(out);

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
  pFileHead->sizeCompressed = pLocalHead->sizeCompressed;
  pFileHead->sizeUncompressed = pLocalHead->sizeUncompressed;
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
  pDirEnd->centralDirSize = size_cast<uint32_t>(cFileHead);
  pDirEnd->centralDirStart = size_cast<uint32_t>(ofsFileHead);
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
void file::decodeChunk(chunk_t* pChunk)
{
  const unsigned char* out = pChunk->pZipped;
  const unsigned char* p0 = out;

  // xx xx xx xx yy yy yy yy zz zz zz zz 18 ab 01 00 00 00 00 00
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
    printf(" %02x", out[1]);
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

  // Another Height Map (without ground decorations?)
  out += 16 * 16;

  // All 03's Map (biome?, 03 = pine forest)
  out += 16 * 16;

  // 03 00 00 00 0F 00 Map
  // 00 00 00 00 0F 00 Map
  out += 16 * 16 * 6;

  // 00 ff
  out += 2;

  // 00 00
  // 01 00 09 bspd.main ff ff ff ff ff ff ff ff 00 02 00 01 00
  if (*out != 0) {
    out += 25;
  }
  else {
    out += 2;
  }

  // +/- something Map
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

  // const unsigned char* p1 = out;
  // const unsigned char* p2 = p0 + outsize;
  // p1 .. p2: 01 00 00 00 00 00 00 00 00 00 00 01 00
}

// ============================================================================
void file::compareChunks(const chunk_t* pChunk0, const chunk_t* pChunk1)
{
  const uint8_t* ptr0 = pChunk0->pZipped;
  const uint8_t* ptr1 = pChunk1->pZipped;

  // xx xx xx xx yy yy yy yy zz zz zz zz 18 ab 01 00 00 00 00 00
  ptr0 += 0x14;
  ptr1 += 0x14;

  const int sections = 64;
  for (int i = 0; i < sections; i++)
  {
    if (ptr0[0] == 0) {
      ptr0 += 1;
    } else {
      if (ptr0[1] == 0) {
        ptr0 += 2;
        ptr0 += 1;
      }
      else {
        ptr0 += 2;
        ptr0 += 16 * 16 * 4;
      }

      if (ptr0[0] == 0) {
        ptr0 += 1;
      }
      else {
        ptr0 += 1;
        ptr0 += 16 * 16 * 4 * 3;
      }
    }

    if (ptr1[0] == 0) {
      ptr1 += 1;
    }
    else {
      if (ptr1[1] == 0) {
        ptr1 += 2;
        ptr1 += 1;
      }
      else {
        ptr1 += 2;
        ptr1 += 16 * 16 * 4;
      }

      if (ptr1[0] == 0) {
        ptr1 += 1;
      }
      else {
        ptr1 += 1;
        ptr1 += 16 * 16 * 4 * 3;
      }
    }
  }

  // 0f: solid, 00 air
  for (int i = 0; i < sections; i++)
  {
    if (ptr0[0] == 0) {
      ptr0 += 1;
      ptr0 += 16 * 16 * 4;
    }
    else {
      ptr0 += 1;
      ptr0 += 1;
    }

    if (ptr1[0] == 0) {
      ptr1 += 1;
      ptr1 += 16 * 16 * 4;
    }
    else {
      ptr1 += 1;
      ptr1 += 1;
    }
  }

  // Height Map
  ptr0 += 16 * 16;
  ptr1 += 16 * 16;

  // Another Height Map (without ground decorations?)
  ptr0 += 16 * 16;
  ptr1 += 16 * 16;

  // All 03's Map (biome?, 03 = pine forest)
  ptr0 += 16 * 16;
  ptr1 += 16 * 16;

  // 03 0C 00 00 0F 00 Map
  // 03 00 00 00 0F 00 Map
  // 00 00 00 00 0F 00 Map
  ptr0 += 16 * 16 * 6;
  ptr1 += 16 * 16 * 6;

  // 00 ff 00 00
  ptr0 += 2;
  ptr1 += 2;

  // 00 00
  // 01 00 09 bspd.main ff ff ff ff ff ff ff ff 00 02 00 01 00
  if (*ptr0 != 0) {
    ptr0 += 25;
  } else {
    ptr0 += 2;
  }

  ptr1 += 2;

  // +/- something Map
  ptr0 += 16 * 16;
  ptr1 += 16 * 16;

  // 7d/7E Map 
  ptr0 += 16 * 16;
  ptr1 += 16 * 16;

  // +/- something Map
  ptr0 += 16 * 16;
  ptr1 += 16 * 16;

  // fill level? -128 .. 127 (solid .. air)
  for (int i = 0; i < sections; i++)
  {
    if (ptr0[0] == 0) {
      ptr0 += 1;
      ptr0 += 16 * 16 * 4;
    }
    else {
      ptr0 += 1;
      ptr0 += 1;
    }

    if (ptr1[0] == 0) {
      ptr1 += 1;
      ptr1 += 16 * 16 * 4;
    }
    else {
      ptr1 += 1;
      ptr1 += 1;
    }
  }

  // 00: solid, 0f: air
  for (int i = 0; i < sections; i++)
  {
    if (ptr0[0] == 0) {
      ptr0 += 1;
      ptr0 += 16 * 16 * 4;
    }
    else {
      ptr0 += 1;
      ptr0 += 1;
    }

    if (ptr1[0] == 0) {
      ptr1 += 1;
      ptr1 += 16 * 16 * 4;
    }
    else {
      ptr1 += 1;
      ptr1 += 1;
    }
  }

  // all 0
  for (int i = 0; i < sections; i++)
  {
    if (ptr0[0] == 0) {
      ptr0 += 1;
      ptr0 += 16 * 16 * 4;
    }
    else {
      ptr0 += 1;
      ptr0 += 1;
    }
    if (ptr1[0] == 0) {
      ptr1 += 1;
      ptr1 += 16 * 16 * 4;
    }
    else {
      ptr1 += 1;
      ptr1 += 1;
    }
  }

  // block damage
  for (int i = 0; i < sections; i++)
  {
    if (ptr0[0] == 0) {
      ptr0 += 1;
      ptr0 += 16 * 16 * 4;
    }
    else {
      ptr0 += 1;
      ptr0 += 1;
    }
    if (ptr1[0] == 0) {
      ptr1 += 1;
      ptr1 += 16 * 16 * 4;
    }
    else {
      ptr1 += 1;
      ptr1 += 1;
    }
  }

  // const unsigned char* p1 = out;
  // const unsigned char* p2 = p0 + outsize;
  // p1 .. p2: 01 00 00 00 00 00 00 00 00 00 00 01 00
}

// ============================================================================
void _encodeAirBlockSection(uint8_t** ptr)
{
  (*ptr)[0] = 0;
  ++(*ptr);
}

// ============================================================================
void _encodeBlockSection(uint8_t** ptr, uint32_t x)
{
  (*ptr)[0] = 1;
  (*ptr)[1] = 0;
  (*ptr)[2] = static_cast<uint8_t>(x & 0xff);
  (*ptr) += 3;
}

// ============================================================================
void _encodeBlockSection(uint8_t** ptr, const uint32_t* p, int section)
{
  p += 4 * 16 * 16 * section;

  (*ptr)[0] = 1;
  (*ptr)[1] = 1;
  (*ptr) += 2;

  for (int i = 0; i < 16 * 16 * 4; i++) {
    (*ptr)[i] = static_cast<uint8_t>(p[i] & 0xff);
  }

  (*ptr) += 16 * 16 * 4;
}

// ============================================================================
void _encodeNoExtendedBlocks(uint8_t** ptr)
{
  (*ptr)[0] = 0;
  ++(*ptr);
}

// ============================================================================
void _encodeExtendedBlockIds(uint8_t** ptr, const uint32_t* p)
{
  (*ptr)[0] = 1;
  ++(*ptr);

  for (int i = 0; i < 16 * 16 * 4 * 3; i+=3) {
    (*ptr)[i  ] = static_cast<uint8_t>((p[i] >>  8) & 0xff);
    (*ptr)[i+1] = static_cast<uint8_t>((p[i] >> 16) & 0xff);
    (*ptr)[i+2] = static_cast<uint8_t>((p[i] >> 24) & 0xff);
  }

  (*ptr) += 16 * 16 * 4 * 3;
}

// ============================================================================
void _encodeRLESectionFill(uint8_t** ptr, uint8_t x)
{
  (*ptr)[0] = 1;
  (*ptr)[1] = x;
  (*ptr) += 2;
}

// ============================================================================
void _encodeRLESectionCopy(uint8_t** ptr, const uint8_t* x, int section)
{
  x += 4 * 16 * 16 * section;

  (*ptr)[0] = 0;
  ++(*ptr);

  memcpy(*ptr, x, 16 * 16 * 4);
  (*ptr) += 16 * 16 * 4;
}

// ============================================================================
void file::encodeChunk(chunk_t** ppChunk, int x, int z, int rx, int rz)
{
  chunk_t* pChunk = new chunk_t(x, z);
  pChunk->c7rg = 16 * 16 * 256 * 8;
  pChunk->p7rg = new uint8_t[pChunk->c7rg];
  memset(pChunk->p7rg, 0, pChunk->c7rg);

  const size_t cZipped = 16 * 16 * 256 * 8;
  uint8_t* pZipped = new uint8_t[cZipped];
  memset(pZipped, 0, cZipped);

  pChunk->pZipped = pZipped;

  uint8_t* p0 = pZipped;
  uint8_t* ptr = pZipped;

  // xx xx xx xx yy yy yy yy zz zz zz zz 18 ab 01 00 00 00 00 00
  int* pCoords = reinterpret_cast<int*>(ptr);
  int tx = (rx < 0) ? ((x + 1) & 31) : (x);
  int tz = (rz < 0) ? ((z + 1) & 31) : (z);

  int cx = 32 * rx + tx;
  int cz = 32 * rz + tz;

  pCoords[0] = cx;
  pCoords[1] = 0;
  pCoords[2] = cz;
  pCoords[3] = 0;
  pCoords[4] = 0;
  ptr += 0x14;

  const int sections = 64;
  _encodeBlockSection(&ptr, 12);
  _encodeNoExtendedBlocks(&ptr);

  _encodeBlockSection(&ptr, 1);
  _encodeNoExtendedBlocks(&ptr);

  for (int i = 2; i < sections; i++) {
    _encodeAirBlockSection(&ptr); // 00
  }

  // 0f: solid, 00 air
  _encodeRLESectionFill(&ptr, 0x0f);
  _encodeRLESectionFill(&ptr, 0x0f);
  for (int i = 2; i < sections; i++)
  {
    _encodeRLESectionFill(&ptr, 0x00);
  }

  // Height Map
  memset(ptr, 8, 16*16);
  ptr += 16 * 16;

  // Another Height Map (without ground decorations?)
  memset(ptr, 8, 16*16);
  ptr += 16 * 16;

  // All 03's Map (biome?, 03 = pine forest)
  memset(ptr, 3, 16*16);
  ptr += 16 * 16;

  // 03 00 00 00 0F 00 Map
  // 00 00 00 00 0F 00 Map
  for (int i = 0; i < 16*16*6; i+=6) {
    ptr[i  ] = 0x00;
    ptr[i+1] = 0x00;
    ptr[i+2] = 0x00;
    ptr[i+3] = 0x00;
    ptr[i+4] = 0x0f;
    ptr[i+5] = 0x00;
  }
  ptr += 16 * 16 * 6;

  // ??
  ptr[0] = 0x00;
  ptr[1] = 0xff;
  ptr[2] = 0x00;
  ptr[3] = 0x00;
  ptr += 4;

  // +/- something Map
  memset(ptr, 0, 16*16);
  ptr += 16 * 16;

  // 7d/7E Map 
  memset(ptr, 0x7d, 16*16);
  ptr += 16 * 16;

  // +/- something Map
  memset(ptr, 0, 16*16);
  ptr += 16 * 16;

  // fill level? -128 .. 127 (solid .. air)
  _encodeRLESectionFill(&ptr, 0x80);
  _encodeRLESectionFill(&ptr, 0x80);
  for (int i = 2; i < sections; i++)
  {
    _encodeRLESectionFill(&ptr, 0x7f);
  }

  // 00: solid, 0f: air, 0a: ??
  _encodeRLESectionFill(&ptr, 0x00);
  _encodeRLESectionFill(&ptr, 0x00);
  for (int i = 2; i < sections; i++)
  {
    _encodeRLESectionFill(&ptr, 0x0f);
  }

  // all 0
  for (int i = 0; i < sections; i++)
  {
    _encodeRLESectionFill(&ptr, 0x00);
  }

  // block damage
  for (int i = 0; i < sections; i++)
  {
    _encodeRLESectionFill(&ptr, 0x00);
  }

  // const unsigned char* p1 = out;
  // const unsigned char* p2 = p0 + outsize;
  // p1 .. p2: 01 00 00 00 00 00 00 00 00 00 00 01 00
  const uint8_t Footer[] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
  };

  memcpy(ptr, Footer, sizeof(Footer));
  ptr += sizeof(Footer);

  pChunk->cZipped = ptr - p0;

  *ppChunk = pChunk;
}

// ============================================================================
static void createChunkBlockFromHeightMap(uint32_t** ppBlocks, const world::world_t* pWorld, int cx, int cz)
{
  uint32_t* pBlocks = new uint32_t [16*16*256];
  *ppBlocks = pBlocks;

  for (int z = 0; z < 16; ++z) {
    for (int x = 0; x < 16; ++x) {
      float height = pWorld->height[16*cz+z][16*cx+x];
      int iHeight = static_cast<int>(height);
      for (int y = 0; y < 256; ++y) {
        uint32_t blockId = 0;
        if (y < iHeight) { blockId = 1; }
        pBlocks[x+16*z+16*16*y] = blockId;
      }
    }
  }
}

// ============================================================================
static void createChunkAirFromHeightMap(uint8_t** ppBlocks, const world::world_t* pWorld, int cx, int cz)
{
  uint8_t* pBlocks = new uint8_t[16*16*256];
  *ppBlocks = pBlocks;

  for (int z = 0; z < 16; ++z) {
    for (int x = 0; x < 16; ++x) {
      float height = pWorld->height[16*cz+z][16*cx+x];
      int iHeight = static_cast<int>(height);
      for (int y = 0; y < 256; ++y) {
        uint32_t airId = 0;
        if (y < iHeight) { airId = 0x0f; }
        pBlocks[x+16*z+16*16*y] = airId;
      }
    }
  }
}

// ============================================================================
static void createChunkNonAirFromHeightMap(uint8_t** ppBlocks, const world::world_t* pWorld, int cx, int cz)
{
  uint8_t* pBlocks = new uint8_t[16*16*256];
  *ppBlocks = pBlocks;

  for (int z = 0; z < 16; ++z) {
    for (int x = 0; x < 16; ++x) {
      float height = pWorld->height[16*cz+z][16*cx+x];
      int iHeight = static_cast<int>(height);
      for (int y = 0; y < 256; ++y) {
        uint32_t nonAirId = 0x0f;
        if (y < iHeight) { nonAirId = 0x00; }
        pBlocks[x+16*z+16*16*y] = nonAirId;
      }
    }
  }
}

// ============================================================================
static void createChunkLevelFromHeightMap(uint8_t** ppBlocks, const world::world_t* pWorld, int cx, int cz)
{
  uint8_t* pBlocks = new uint8_t[16*16*256];
  *ppBlocks = pBlocks;

  for (int z = 0; z < 16; ++z) {
    for (int x = 0; x < 16; ++x) {
      float height = pWorld->height[16*cz+z][16*cx+x];
      for (int y = 0; y < 256; ++y) {
        float d = std::min(std::max((y - height)*128.0f, -128.0f), 127.0f);
        uint8_t level = static_cast<uint8_t>(d);
        pBlocks[x+16*z+16*16*y] = level;
      }
    }
  }
}

// ============================================================================
static void createHeightMapFromHeightMap(uint8_t** ppMap, const world::world_t* pWorld, int cx, int cz)
{
  uint8_t* pMap = new uint8_t[16*16];
  *ppMap = pMap;

  for (int z = 0; z < 16; ++z) {
    for (int x = 0; x < 16; ++x) {
      float height = pWorld->height[16*cz+z][16*cx+x];
      int iHeight = static_cast<int>(height);
      pMap[x+16*z] = std::min(std::max(iHeight, 4), 255) - 1;
    }
  }
}

// ============================================================================
template <typename T>
static bool allBlocksEqual(const T* pBlocks, int section, T* pBlockId)
{
  pBlocks += section * 4 * 16 * 16;
  T blockId = pBlocks[0];
  for (int z = 0; z < 16; ++z) {
    for (int x = 0; x < 16; ++x) {
      for (int y = 0; y < 4; ++y) {
        if (blockId != pBlocks[x+16*z+16*16*y]) {
          return false;
        }
      }
    }
  }

  *pBlockId = blockId;
  return true;
}

// ============================================================================
void file::encodeChunk(chunk_t** ppChunk, int x, int z, int rx, int rz, const world::world_t* pWorld)
{
  chunk_t* pChunk = new chunk_t(x, z);
  pChunk->c7rg = 16 * 16 * 256 * 8;
  pChunk->p7rg = new uint8_t[pChunk->c7rg];
  memset(pChunk->p7rg, 0, pChunk->c7rg);

  const size_t cZipped = 16 * 16 * 256 * 8;
  uint8_t* pZipped = new uint8_t[cZipped];
  memset(pZipped, 0, cZipped);

  pChunk->pZipped = pZipped;

  uint8_t* p0 = pZipped;
  uint8_t* ptr = pZipped;

  // xx xx xx xx yy yy yy yy zz zz zz zz 18 ab 01 00 00 00 00 00
  int* pCoords = reinterpret_cast<int*>(ptr);
  int tx = (rx < 0) ? ((x + 1) & 31) : (x);
  int tz = (rz < 0) ? ((z + 1) & 31) : (z);

  int cx = 32 * rx + tx;
  int cz = 32 * rz + tz;

  pCoords[0] = cx;
  pCoords[1] = 0;
  pCoords[2] = cz;
  pCoords[3] = 0;
  pCoords[4] = 0;
  ptr += 0x14;

  const int sections = 64;

  uint32_t* pBlocks = nullptr;
  createChunkBlockFromHeightMap(&pBlocks, pWorld, cx, cz);

  _encodeBlockSection(&ptr, 12);
  _encodeNoExtendedBlocks(&ptr);

  for (int i = 1; i < sections; i++)
  {
    uint32_t blockId = 0;
    if (allBlocksEqual(pBlocks, i, &blockId)) {
      if (blockId == 0) {
        _encodeAirBlockSection(&ptr);
      } else {
        _encodeBlockSection(&ptr, blockId);
        _encodeNoExtendedBlocks(&ptr);
      }
    } else {
      _encodeBlockSection(&ptr, pBlocks, i);
      _encodeNoExtendedBlocks(&ptr);
    }
  }

  delete[] pBlocks;

  // 0f: solid, 00 air
  uint8_t* pAir = nullptr;
  createChunkAirFromHeightMap(&pAir, pWorld, cx, cz);

  _encodeRLESectionFill(&ptr, 0x0f);

  for (int i = 1; i < sections; i++)
  {
    uint8_t blockId = 0;
    if (allBlocksEqual(pAir, i, &blockId)) {
      _encodeRLESectionFill(&ptr, blockId);
    }
    else {
      _encodeRLESectionCopy(&ptr, pAir, i);
    }
  }

  delete pAir;
  pAir = nullptr;

  // Height Map
  uint8_t* pMap = nullptr;
  createHeightMapFromHeightMap(&pMap, pWorld, cx, cz);

  memcpy(ptr, pMap, 16*16);
  ptr += 16 * 16;

  // Another Height Map (without ground decorations?)
  memcpy(ptr, pMap, 16*16);
  ptr += 16 * 16;

  // All 03's Map (biome?, 03 = pine forest)
  memset(ptr, 3, 16*16);
  ptr += 16 * 16;

  delete[] pMap;
  pMap = nullptr;

  // 03 00 00 00 0F 00 Map
  // 00 00 00 00 0F 00 Map
  for (int i = 0; i < 16*16*6; i+=6) {
    ptr[i] = 0x03;
    ptr[i+1] = 0x00;
    ptr[i+2] = 0x00;
    ptr[i+3] = 0x00;
    ptr[i+4] = 0xff;
    ptr[i+5] = 0x00;
  }
  ptr += 16 * 16 * 6;

  // ??
  ptr[0] = 0x00;
  ptr[1] = 0xff;
  ptr[2] = 0x00;
  ptr[3] = 0x00;
  ptr += 4;

  // +/- something Map
  memset(ptr, 0, 16*16);
  ptr += 16 * 16;

  // 7d/7E Map 
  memset(ptr, 0x7d, 16*16);
  ptr += 16 * 16;

  // +/- something Map
  memset(ptr, 0, 16*16);
  ptr += 16 * 16;

  // fill level? -128 .. 127 (solid .. air)
  uint8_t* pLevel = nullptr;
  createChunkLevelFromHeightMap(&pLevel, pWorld, cx, cz);

  _encodeRLESectionFill(&ptr, 0x80);
  for (int i = 1; i < sections; i++)
  {
    uint8_t level = 0;
    if (allBlocksEqual(pLevel, i, &level)) {
      _encodeRLESectionFill(&ptr, level);
    }
    else {
      _encodeRLESectionCopy(&ptr, pLevel, i);
    }
  }

  delete[] pLevel;
  pLevel = nullptr;

  // 00: solid, 0f: air, 0a: ??
  uint8_t* pNonAir = nullptr;
  createChunkNonAirFromHeightMap(&pNonAir, pWorld, cx, cz);

  _encodeRLESectionFill(&ptr, 0x00);

  for (int i = 1; i < sections; i++)
  {
    uint8_t blockId = 0;
    if (allBlocksEqual(pNonAir, i, &blockId)) {
      _encodeRLESectionFill(&ptr, blockId);
    }
    else {
      _encodeRLESectionCopy(&ptr, pNonAir, i);
    }
  }

  delete[] pNonAir;
  pNonAir = nullptr;

  // all 0
  for (int i = 0; i < sections; i++) {
    _encodeRLESectionFill(&ptr, 0x00);
  }

  // block damage
  for (int i = 0; i < sections; i++) {
    _encodeRLESectionFill(&ptr, 0x00);
  }

  // const unsigned char* p1 = out;
  // const unsigned char* p2 = p0 + outsize;
  // p1 .. p2: 01 00 00 00 00 00 00 00 00 00 00 01 00
  const uint8_t Footer[] ={
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0
  };

  memcpy(ptr, Footer, sizeof(Footer));
  ptr += sizeof(Footer);

  pChunk->cZipped = ptr - p0;

  *ppChunk = pChunk;
}

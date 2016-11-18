#include "main.h"

#include "gfx.h"
#include "BMPTypes.h"

static void convertToBitmapInfoHeader(const gfx::imagegeo_t&, BitmapInfoHeader*);

// ============================================================================
void gfx::readBMPFileInfo(const char* FName, imagegeo_t* pInfo)
{
  FILE* hFile = fopen(FName, "rb");
  if (hFile == 0) {
    throw;
  }

  BitmapFileHeader bmfh = { 0 };
  BitmapInfoHeader bmih = { 0 };

	size_t BytesBMFH = fread(&bmfh, 1, sizeof(BitmapFileHeader), hFile);
  size_t BytesBMIH = fread(&bmih, 1, sizeof(BitmapInfoHeader), hFile);

  fclose(hFile);

	if  (BytesBMFH != sizeof(BitmapFileHeader)
    || BytesBMIH != sizeof(BitmapInfoHeader)
    || bmfh.bfType != 'MB')
  {
    throw;
	}

  pInfo->cf = cfRGB0888;
  pInfo->cx = bmih.biWidth;
  pInfo->cy = bmih.biHeight;
}

// ============================================================================
void gfx::readBMPFileData(const char* FName, image_t* pImage)
{
  FILE* hFile = fopen(FName, "rb");
  if (hFile == 0) {
    throw;
  }

  BitmapFileHeader bmfh = { 0 };
  BitmapInfoHeader bmih = { 0 };

  size_t BytesBMFH = fread(&bmfh, 1, sizeof(BitmapFileHeader), hFile);
  size_t BytesBMIH = fread(&bmih, 1, sizeof(BitmapInfoHeader), hFile);

  if  (BytesBMFH != sizeof(BitmapFileHeader)
    || BytesBMIH != sizeof(BitmapInfoHeader)
    || bmfh.bfType != 'MB')
  {
    fclose(hFile);
    throw;
  }

	unsigned LineSize = (((bmih.biWidth * bmih.biBitCount) / 8) + 3) & (~3);

  uint32_t Palette[256] = { 0 };
	size_t BytesPAL = fread(Palette, 1, 4*bmih.biClrUsed, hFile);
	if (BytesPAL != 4*bmih.biClrUsed) {
    throw;
	}

  fseek(hFile, bmfh.bfOffBits, SEEK_SET);

	uint8_t* pLine = pImage->data[0] + pImage->stride.p[0] * (bmih.biHeight-1);

	for (uint32_t y = 0; y < bmih.biHeight; y++,pLine-=pImage->stride.p[0])
	{
		size_t Bytes = fread(pLine, 1, LineSize, hFile);
		if (Bytes != LineSize) {
      break;
		}
	}

	fclose(hFile);
}

// ============================================================================
void gfx::saveBMPFile(const char* FName, const imagegeo_t& geo, const image_t& img, bool Overwrite)
{
  FILE* hFile = fopen(FName, Overwrite ? "w+b" : "wb");
  if (hFile == 0) {
    throw;
  }

  BitmapInfoHeader bmih = { 0 };
  convertToBitmapInfoHeader(geo, &bmih);

  BitmapFileHeader bmfh = { 0 };
  bmfh.bfSize = sizeof(BitmapFileHeader);
  bmfh.bfType = 'MB';
  bmfh.bfOffBits = bmfh.bfSize + bmih.biSize + 4*bmih.biClrUsed;

  size_t BytesBMFH = fwrite(&bmfh, 1, sizeof(BitmapFileHeader), hFile);
  size_t BytesBMIH = fwrite(&bmih, 1, sizeof(BitmapInfoHeader), hFile);

  if  (BytesBMFH != sizeof(BitmapFileHeader)
    || BytesBMIH != sizeof(BitmapInfoHeader)) {
    throw;
  }

  // write palette
  if (geo.cf == cfY8)
  {
    uint32_t Palette[256];
    for (int y = 0; y < 256; y++) {
      Palette[y] = (y) | (y<<8) | (y<<16);
    }

    size_t Bytes = fwrite(Palette, 1, 4*bmih.biClrUsed, hFile);
    if (Bytes != 4*bmih.biClrUsed) {
      throw;
    }
  }

  // now write the lines to the disk
  const uint8_t* pSrc = (const uint8_t*)img.data[0] + img.stride.p[0] * (geo.cy-1);
  int bmpStride = (((bmih.biWidth * bmih.biBitCount) / 8) + 3) & (~3);

  for (unsigned y = 0; y < geo.cy; y++, pSrc -= img.stride.p[0])
  {
    size_t Bytes = fwrite(pSrc, 1, bmpStride, hFile);
    if (Bytes != static_cast<size_t>(bmpStride)) {
      throw;
    }
  }

  fclose(hFile);
}

// ----------------------------------------------------------------------------
static void convertToBitmapInfoHeader(const gfx::imagegeo_t& geo, BitmapInfoHeader* pbmih)
{
  memset(pbmih, 0, sizeof(BitmapInfoHeader));

  pbmih->biSize = sizeof(BitmapInfoHeader);
  pbmih->biWidth = geo.cx;
  pbmih->biHeight = geo.cy;
  pbmih->biPlanes = 1;

  switch (geo.cf)
  {
  case gfx::cfY8:
    pbmih->biBitCount = 8;
    pbmih->biClrUsed = 256;
    break;

  case gfx::cfRGB0888:
    pbmih->biBitCount = 24;
    break;

  case gfx::cfRGB8888:
    pbmih->biBitCount = 32;
    break;
  }

  int bmpStride = (((pbmih->biWidth * pbmih->biBitCount) / 8) + 3) & (~3);
  pbmih->biSizeImage = bmpStride * pbmih->biHeight;
}

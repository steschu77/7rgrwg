#pragma once

#pragma pack(push, 1)

// ============================================================================
struct BitmapFileHeader
{
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
};

// ============================================================================
struct BitmapInfoHeader
{
  uint32_t biSize;
  uint32_t biWidth;
  uint32_t biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  long biXPelsPerMeter;
  long biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
};

#pragma pack(pop)

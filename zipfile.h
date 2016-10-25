#pragma once

// ============================================================================
enum ZipSignature
{
  zipsigFileHeader      = 0x02014b50,
  zipsigLocalFileHeader = 0x04034b50,
  zipsigCentralDirEnd   = 0x06054b50,
  zipsigDataDescriptor  = 0x08074b50,
};

// ============================================================================
enum ZipHostSystem
{
  zipsysFatFileSystem   = 0,
  zipsysAmiga           = 1,
  zipsysVms             = 2,
  zipsysUnix            = 3,
  zipsysVmCms           = 4,
  zipsysAtari           = 5,
  zipsysHpfs            = 6,
  zipsysMacintosh       = 7,
  zipsysZSystem         = 8,
  zipsysCpM             = 9,
  zipsysTops20          = 10,
  zipsysNtfs            = 11,
  zipsysSmsQdos         = 12,
  zipsysAcornRiscOs     = 13,
  zipsysVFat            = 14,
  zipsysMvs             = 15,
  zipsysBeOs            = 16,
  zipsysTandem          = 17,
  zipsysMax             = 0xff
};

// ============================================================================
struct ZipVersion
{
  uint8_t hostSystem;   //!< see enum ZipHostSystem
  uint8_t zipVersion;   //!< x.y with x = ZipVersion / 10, y = ZipVersion % 10
};

// ============================================================================
enum ZipCompressionMethod
{
  zipcompStored          = 0,
  zipcompShrunk          = 1,
  zipcompReduced1        = 2,
  zipcompReduced2        = 3,
  zipcompReduced3        = 4,
  zipcompReduced4        = 5,
  zipcompImploded        = 6,
  zipcompTokenize        = 7,
  zipcompDeflated        = 8,
  zipcompEnhancedDeflate = 9,
  zipcompPkWareImplode   = 10,
  zipcompMax             = 0xffff
};

#pragma pack(push, 1)
// ============================================================================
struct ZipLocalFileHeader
{
  uint32_t signature;
  ZipVersion zVersionNeeded;
  uint16_t flags;
  uint16_t compressionMethod;   //!< see enum ZipCompressionMethod
  uint16_t fileTime;        //!< MS-DOS format
  uint16_t fileDate;        //!< MS-DOS format
  uint32_t crc32;
  uint32_t sizeCompressed;
  uint32_t sizeUncompressed;
  uint16_t fileNameLength;
  uint16_t extraFieldLength;
};

// ============================================================================
struct ZipFileHeader
{
  uint32_t signature;
  ZipVersion zVersionMadeBy;
  ZipVersion zVersionNeeded;
  uint16_t flags;
  uint16_t compressionMethod;   //!< see enum ZipCompressionMethod
  uint16_t fileTime;        //!< MS-DOS format
  uint16_t fileDate;        //!< MS-DOS format
  uint32_t crc32;
  uint32_t sizeCompressed;
  uint32_t sizeUncompressed;
  uint16_t fileNameLength;
  uint16_t extraFieldLength;
  uint16_t fileCommentLength;
  uint16_t startDisk;
  uint16_t fileAttribInternal;
  uint32_t fileAttribExternal;
  uint32_t ofsLocalHeader;
};

// ============================================================================
struct ZipCentralDirEnd
{
  uint32_t signature;
  uint16_t thisDiscNo;
  uint16_t discStart;
  uint16_t thisCentralDirEntries;
  uint16_t centralDirEntries;
  uint32_t centralDirSize;
  uint32_t centralDirStart;
  uint16_t zipFileCommentLength;
};
#pragma pack(pop)

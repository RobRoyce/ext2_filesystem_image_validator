#pragma once

#include <sys/types.h>
#include "imagereader.hpp"

// -------------------------------------------------- Meta File Info
struct MetaFileLimits {
  // Limits enforced by block size
  // Based on Table 2.1 at
  // https://www.nongnu.org/ext2-doc/ext2.html#behaviour-flags
  unsigned int fileSystemBlocks;
  unsigned int blocksPerBlockGroup;
  unsigned int inodesPerBlockGroup;
  unsigned int bytesPerBlockGroup;
  unsigned long fileSystemSizeReal;
  unsigned long fileSystemSizeLinux;
  unsigned long blocksPerFile;
  unsigned long long fileSizeReal;
  unsigned long long fileSizeLinux;
};

/**
   MetaFile contains pertinent data fields regarding the actual EXT2 file
   system. There are 2 levels of meta here, each with their own fields:

   1 -- The actual file we are reading
     -- stat, filename
   2 -- The interpretation of that file
     -- else (not stat, filename)

     Level 1 can be established without reading the superBlock, whereas level 2
     fields require fields from the superBlock. Level 2 fields are essentially
     for convenience, since they can easily be retrieved from the superBlock or
     the meta stat. Note that level 2 fields come before level 1 fields, due to
     the fact that level 1 fields (especially the filename) are variable in
     length, and so belong at the end of the struct to ensure optimal memory
     footprint.
 */
struct MetaFile {
 public:
  // Level 2 variables
  __u32 blockCount;
  __u32 blockSize;
  __u32 inodeSize;
  __u32 inodesPerGroup;
  __u32 blocksPerGroup;
  __u32 blockGroupsCount;
  __u32 groupDescBlockSize;
  __u32 rev;
  unsigned long long blockGroupSize;
  struct stat stat;
  std::string filename;

  // Methods
  MetaFile() {}
  ~MetaFile() {}

private:
  MetaFileLimits limits;
};
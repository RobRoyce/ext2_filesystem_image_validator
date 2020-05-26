#pragma once

#include <fstream>

#include "imagereader.hpp"

// -------------------------------------------------- EXT2 Image Reader Class
//
// Reads an EXT2 Image from standard File I/O into buffers.
//
// Better suited to large images that may not fit into memory.
//
class BufferedImageReader : public ImageReader {
 public:
  BufferedImageReader(MetaFile*);
  ~BufferedImageReader();

  virtual void init();

  struct ext2_super_block *getSuperBlock();

  virtual void *getBlock(size_t);

  virtual void *getBlockGroup(size_t);

  virtual void *getGroupDescriptor();

protected:

  virtual int readSuperBlock();

private:
  
  std::ifstream *fs;

  char *blockBuffer;

  char *blockGroupBuffer;

  char *groupDescriptorBuffer;

  const __u32 GDSIZE = sizeof(ext2_group_desc);
  // Descriptor Table is located at block 1 if block size is 1KiB, otherwise block 2
  const __u32 DESC_TABLE_BLOCK = (meta->blockSize == KiB) ? 3 : 2;
  const __u32 DESC_TABLE_LEN = meta->blockGroupsCount;
  const __u32 DESC_TABLE_SZ = DESC_TABLE_LEN * GDSIZE; // each GD is 32 bytes
  const unsigned GD_BUFLEN = (GDSIZE * DESC_TABLE_LEN) / sizeof(char);

};
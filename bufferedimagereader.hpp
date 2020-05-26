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

  virtual void *getBlock(size_t blockIdx);

  virtual void *getBlocks(size_t blockIdx, size_t numBlocks);

  virtual void *getBlockGroup(size_t blockGroupIdx);

  virtual void *getGroupDescriptor();

protected:

  virtual int readSuperBlock();

private:
  
  std::ifstream *fs;

  char *blockBuffer = nullptr;

  char *blockGroupBuffer = nullptr;

  char *groupDescriptorBuffer = nullptr;

  char *multiBlockBuffer = nullptr;

  /*The total number of blocks that can fit in the multiBlockBuffer*/
  size_t multiBlockBufferCount;

};
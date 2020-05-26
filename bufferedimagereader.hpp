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

};
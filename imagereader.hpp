#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <string>

#include "ext2_fs.h"
#include "metafile.hpp"

extern int debug;

// -------------------------------------------------- EXT2 Image Reader Class
//
// Provides an interface for retrieving data from an EXT2 image file
//
class ImageReader {
 public:
  ImageReader(MetaFile*);

  virtual void init() = 0;

  struct ext2_super_block *getSuperBlock();

  /*Returns a buffer containing the raw data from the specified blockIdx*/
  virtual void *getBlock(size_t blockIdx) = 0;

  /*Returns a buffer containing the raw data from numBlocks contiguous blocks, starting at blockIdx*/
  virtual void *getBlocks(size_t blockIdx, size_t numBlocks) = 0;

  virtual void *getBlockGroup(size_t) = 0;

  virtual void *getGroupDescriptor() = 0;

  static const size_t KiB=1024;

protected:
 
  struct ext2_super_block superBlock;
  struct ext2_dir_entry *ext2DirEntry;

  MetaFile *meta = nullptr;

  virtual int readSuperBlock() = 0;
};
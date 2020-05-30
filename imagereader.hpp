// NAME: Rob Royce, Tyler Hackett
// EMAIL: robroyce1@ucla.edu, tjhackett@ucla.edu
// ID: 705357270, 405180956
#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <memory>

#include "ext2_fs.h"
#include "metafile.hpp"

using std::shared_ptr;

extern int debug;

// -------------------------------------------------- EXT2 Image Reader Class
//
// Provides an interface for retrieving data from an EXT2 image file
//
class ImageReader {
 public:

  // BlockPersistenceType values specify how memory management
  // will be handled when getBlock() is called.
  enum class BlockPersistenceType
  {
    TEMPORARY, // The returned buffer is valid until the next call to getBlock()
    SHARED     // The buffer is shared by any calls that request the same block
  };

  ImageReader(MetaFile*);

  virtual void init() = 0;

  struct ext2_super_block *getSuperBlock();

  /*Returns a buffer containing the raw data from the specified blockIdx*/
  virtual shared_ptr<char[]> getBlock(size_t blockIdx, BlockPersistenceType t = BlockPersistenceType::TEMPORARY) = 0;

  /*Returns a buffer containing the raw data from numBlocks contiguous blocks, starting at blockIdx*/
  virtual shared_ptr<char[]> getBlocks(size_t blockIdx, size_t numBlocks) = 0;

  virtual shared_ptr<char[]> getGroupDescriptor() = 0;

  static const size_t KiB=1024;

protected:
 
  struct ext2_super_block superBlock;
  struct ext2_dir_entry *ext2DirEntry;

  MetaFile *meta = nullptr;

  virtual int readSuperBlock() = 0;
};

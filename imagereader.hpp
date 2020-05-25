#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <string>

#include "ext2_fs.h"

// -------------------------------------------------- EXT2 Image Reader Class
//
// Provides an interface for retrieving data from an EXT2 image file
//
class ImageReader {
 public:
  ImageReader(char*);

  virtual int init() = 0;

  struct ext2_super_block &getSuperBlock();

  virtual void *getBlock(size_t) = 0;

  virtual void *getBlockGroup(size_t) = 0;

  static const size_t KiB=1024;

protected:
 
  struct ext2_super_block superBlock;
  struct ext2_dir_entry *ext2DirEntry;

  struct stat metaFileStat;
  std::string metaFileName;

  __u32 rev;

  virtual int readSuperBlock() = 0;
};
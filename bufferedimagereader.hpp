// NAME: Rob Royce, Tyler Hackett
// EMAIL: robroyce1@ucla.edu, tjhackett@ucla.edu
// ID: 705357270, 405180956
#pragma once
#include <fstream>
#include <map>
#include "imagereader.hpp"

using std::runtime_error;
using std::make_shared;
using std::weak_ptr;
using std::map;

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

  virtual shared_ptr<char[]> getBlock(size_t blockIdx, BlockPersistenceType t);

  virtual shared_ptr<char[]> getBlocks(size_t blockIdx, size_t numBlocks);

  virtual shared_ptr<char[]> getGroupDescriptor();

protected:

  virtual int readSuperBlock();

private:

  std::ifstream *fs;

  shared_ptr<char[]> blockBuffer = nullptr;

  shared_ptr<char[]> groupDescriptorBuffer = nullptr;

  shared_ptr<char[]> multiBlockBuffer = nullptr;

  map<size_t, weak_ptr<char[]>> manualBlockBuffers;

  /*The total number of blocks that can fit in the multiBlockBuffer*/
  size_t multiBlockBufferCount;

};

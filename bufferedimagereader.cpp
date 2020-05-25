#include "bufferedimagereader.hpp"

#include <sys/types.h>
#include "ext2_fs.h"
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <string.h>
#include <sstream>
#include <sys/stat.h>

BufferedImageReader::BufferedImageReader(char *filename) : ImageReader(filename) {}

int BufferedImageReader::init()
{
    this->fs = new std::ifstream(this->metaFileName, std::ios::binary | std::ios::in);

    this->readSuperBlock();

    this->blockSize = this->superBlock.s_log_block_size;
    this->blockGroupSize = this->superBlock.s_log_block_size * this->superBlock.s_blocks_per_group;

    this->blockBuffer = new char[this->blockSize];
    this->blockGroupBuffer = new char[this->blockGroupSize];
}

int BufferedImageReader::readSuperBlock() {
  //std::stringstream buffer;

  if (!fs)
    return -1;

  fs->unsetf(std::ios::skipws);
  fs->seekg(KiB, std::ios::beg);
  fs->read((char*)&superBlock, KiB);

  //if(debug)
  //  this->printSuperBlock();

  // -- Revision determines algorithm for lookup (hash map vs linked list)
  // -- Revision 1:
  // --- variable inode sizes
  // --- extended attributesa
  // --- Sparse SuperBlocks
  
  this->rev = this->superBlock.s_rev_level;

  return 0;
}

void *BufferedImageReader::getBlock(size_t blockIdx)
{
  if (!fs)
    return nullptr; // TODO Throw exception instead of return nullptr

  fs->unsetf(std::ios::skipws);
  fs->seekg(blockIdx * this->superBlock.s_log_block_size, std::ios::beg);
  fs->read(this->blockBuffer, this->superBlock.s_log_block_size);

  return this->blockBuffer;
}

void *BufferedImageReader::getBlockGroup(size_t blockGroupIdx)
{
  if (!fs)
    return nullptr; // TODO Throw exception instead of return nullptr

  fs->unsetf(std::ios::skipws);
  fs->seekg((blockGroupIdx * this->blockGroupSize) + KiB, std::ios::beg);
  fs->read(this->blockGroupBuffer, this->superBlock.s_log_block_size);

  return this->blockGroupBuffer;
}
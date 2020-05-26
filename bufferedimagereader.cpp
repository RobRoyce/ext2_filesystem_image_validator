#include "bufferedimagereader.hpp"

#include <sys/types.h>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <string.h>
#include <sstream>
#include <sys/stat.h>

BufferedImageReader::BufferedImageReader(MetaFile *metafile) : ImageReader(metafile) {}

void BufferedImageReader::init()
{
    this->fs = new std::ifstream(this->meta->filename, std::ios::binary | std::ios::in);

    this->readSuperBlock();

    this->blockSize = KiB << this->superBlock.s_log_block_size;
    this->blockGroupSize = this->blockSize * this->superBlock.s_blocks_per_group;

    this->blockBuffer = new char[this->blockSize];
    this->blockGroupBuffer = new char[this->blockGroupSize];

    this->groupDescriptorBuffer = new char[KiB];
}

int BufferedImageReader::readSuperBlock() {

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
  
  this->meta->rev = this->superBlock.s_rev_level;

  return 0;
}

void *BufferedImageReader::getBlock(size_t blockIdx)
{
  if (!fs)
    return nullptr; // TODO Throw exception instead of return nullptr

  fs->seekg(blockIdx * this->blockSize, std::ios::beg);
  fs->read(this->blockBuffer, this->blockSize);

  return this->blockBuffer;
}

void *BufferedImageReader::getBlockGroup(size_t blockGroupIdx)
{
  if (!fs)
    return nullptr; // TODO Throw exception instead of return nullptr

  fs->seekg((blockGroupIdx * this->blockGroupSize) + KiB, std::ios::beg);
  fs->read(this->blockGroupBuffer, this->blockGroupSize);

  return this->blockGroupBuffer;
}

void *BufferedImageReader::getGroupDescriptor()
{
  // Descriptor Table is located at block 1 if block size is 1KiB, otherwise block 2
  const __u32 GDSIZE = sizeof(ext2_group_desc);
  const __u32 DESC_TABLE_BLOCK = (meta->blockSize == KiB) ? 3 : 2;
  const __u32 DESC_TABLE_LEN = meta->blockGroupsCount;
  const __u32 DESC_TABLE_SZ = DESC_TABLE_LEN * GDSIZE; // each GD is 32 bytes
  const unsigned BUFLEN = (GDSIZE * DESC_TABLE_LEN) / sizeof(char);

  if (debug) {
    printf("Group Descriptor Size: %d...\n", GDSIZE);
    printf("Location of first Group Descriptor Table: Block %d...\n", DESC_TABLE_BLOCK);
    printf("Total number of Group Descriptors: %d...\n", DESC_TABLE_LEN);
    printf("Total size of Group Descriptor Table: %d...\n", DESC_TABLE_SZ);
    printf("Size (in bytes) of buffer being used: %d...\n", BUFLEN);
  }

  fs->seekg(DESC_TABLE_BLOCK * meta->blockSize, std::ios::beg);
  fs->read(this->groupDescriptorBuffer, DESC_TABLE_SZ);
  
  return this->groupDescriptorBuffer;
}

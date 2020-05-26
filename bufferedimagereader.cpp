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

BufferedImageReader::~BufferedImageReader() 
{
  delete[] blockBuffer;
  delete[] blockGroupBuffer;
  delete[] groupDescriptorBuffer;
}

void BufferedImageReader::init()
{
    this->fs = new std::ifstream(meta->filename, std::ios::binary | std::ios::in);

    this->readSuperBlock();

    this->blockBuffer = new char[meta->blockSize];
    this->blockGroupBuffer = new char[meta->blockGroupSize];

    this->multiBlockBufferCount = 8; // Give the multi-block buffer an arbitrary starting count.
    this->multiBlockBuffer = new char[multiBlockBufferCount * meta->blockSize];
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

  fs->seekg(blockIdx * meta->blockSize, std::ios::beg);
  fs->read(this->blockBuffer, meta->blockSize);

  return this->blockBuffer;
}

void *BufferedImageReader::getBlocks(size_t blockIdx, size_t numBlocks)
{
  if (!fs)
    return nullptr; // TODO Throw exception instead of return nullptr

  // Resize our internal buffer if it is not large enough for the request.
  if(numBlocks > multiBlockBufferCount)
  {
    delete[] multiBlockBuffer;
    multiBlockBufferCount = numBlocks;
    multiBlockBuffer = new char[multiBlockBufferCount * meta->blockSize];
  }

  fs->seekg(blockIdx * meta->blockSize, std::ios::beg);
  fs->read(this->multiBlockBuffer, numBlocks * meta->blockSize);

  return this->multiBlockBuffer;
}

void *BufferedImageReader::getBlockGroup(size_t blockGroupIdx)
{
  if (!fs)
    return nullptr; // TODO Throw exception instead of return nullptr

  fs->seekg((blockGroupIdx * meta->blockGroupSize) + KiB, std::ios::beg);
  fs->read(this->blockGroupBuffer, meta->blockGroupSize);

  return this->blockGroupBuffer;
}

void *BufferedImageReader::getGroupDescriptor()
{
   // Only prepare the buffer once.
  if(!groupDescriptorBuffer)
  {
    const __u32 GDSIZE = sizeof(ext2_group_desc);
    // Descriptor Table is located at block 1 if block size is 1KiB, otherwise block 2
    const __u32 DESC_TABLE_BLOCK = (meta->blockSize == KiB) ? 2 : 1;
    const __u32 DESC_TABLE_LEN = meta->blockGroupsCount;
    const __u32 DESC_TABLE_SZ = DESC_TABLE_LEN * GDSIZE; // each GD is 32 bytes
    const unsigned GD_BUFLEN = (GDSIZE * DESC_TABLE_LEN) / sizeof(char);

    if (!fs)
        return nullptr; // TODO Throw exception instead of return nullptr

    if (debug) {
      printf("Group Descriptor Size: %d...\n", GDSIZE);
      printf("Location of first Group Descriptor Table: Block %d...\n", DESC_TABLE_BLOCK);
      printf("Total number of Group Descriptors: %d...\n", DESC_TABLE_LEN);
      printf("Total size of Group Descriptor Table: %d...\n", DESC_TABLE_SZ);
      printf("Size (in bytes) of buffer being used: %d...\n", GD_BUFLEN);
    }

    groupDescriptorBuffer = new char[GD_BUFLEN];

    fs->seekg(DESC_TABLE_BLOCK * meta->blockSize, std::ios::beg);
    fs->read(groupDescriptorBuffer, DESC_TABLE_SZ);
  }

  return groupDescriptorBuffer;
}

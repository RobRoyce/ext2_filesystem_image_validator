#include "bufferedimagereader.hpp"

#include <sys/types.h>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <string.h>
#include <sstream>
#include <sys/stat.h>

BufferedImageReader::BufferedImageReader(MetaFile *metafile) : ImageReader(metafile)
{
    this->fs = new std::ifstream(meta->filename, std::ios::binary | std::ios::in);

    this->readSuperBlock();
}

BufferedImageReader::~BufferedImageReader() {}

void BufferedImageReader::init()
{
    this->blockBuffer = shared_ptr<char[]>(new char[meta->blockSize]);

    this->multiBlockBufferCount = 8; // Give the multi-block buffer an arbitrary starting count.
    this->multiBlockBuffer = shared_ptr<char[]>(new char[multiBlockBufferCount * meta->blockSize]);
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

shared_ptr<char[]> BufferedImageReader::getBlock(size_t blockIdx, BlockPersistenceType t)
{
  if (!fs)
    throw runtime_error("BufferedImageReader failed to initialize properly, or never initialized in the first place");

  shared_ptr<char[]> buffer;

  switch(t)
  {
    case BlockPersistenceType::TEMPORARY:

      buffer = this->blockBuffer;
      break;

    case BlockPersistenceType::SHARED:

      try {

        weak_ptr<char[]> weakBuf = manualBlockBuffers.at(blockIdx);

        if(weakBuf.expired())
        {
          buffer = shared_ptr<char[]>(new char[meta->blockSize]);
          manualBlockBuffers[blockIdx] = buffer;
        }
        else
        {
          return weakBuf.lock();
        }

      } catch(...) {

        buffer = shared_ptr<char[]>(new char[meta->blockSize]);
        manualBlockBuffers[blockIdx] = buffer;

      }

      break;

    default:
      throw runtime_error("Unsupported BlockPersistenceType");
  }

  fs->seekg(blockIdx * meta->blockSize, std::ios::beg);
  fs->read(buffer.get(), meta->blockSize);

  return buffer;
}

shared_ptr<char[]> BufferedImageReader::getBlocks(size_t blockIdx, size_t numBlocks)
{
  if (!fs)
    throw runtime_error("BufferedImageReader failed to initialize properly, or never initialized in the first place");

  // Resize our internal buffer if it is not large enough for the request.
  if(numBlocks > multiBlockBufferCount)
  {
    multiBlockBufferCount = numBlocks;
    multiBlockBuffer = shared_ptr<char[]>(new char[multiBlockBufferCount * meta->blockSize]);
  }

  fs->seekg(blockIdx * meta->blockSize, std::ios::beg);
  fs->read(this->multiBlockBuffer.get(), numBlocks * meta->blockSize);

  return this->multiBlockBuffer;
}

shared_ptr<char[]> BufferedImageReader::getGroupDescriptor()
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


    if(DESC_TABLE_LEN <= 0 || DESC_TABLE_SZ <= 0)
      throw runtime_error("MalformedDescriptorTable");

    if (!fs)
      throw runtime_error("BufferedImageReader failed to initialize properly, or never initialized in the first place");

    if (debug) {
      printf("Group Descriptor Size: %d...\n", GDSIZE);
      printf("Location of first Group Descriptor Table: Block %d...\n", DESC_TABLE_BLOCK);
      printf("Total number of Group Descriptors: %d...\n", DESC_TABLE_LEN);
      printf("Total size of Group Descriptor Table: %d...\n", DESC_TABLE_SZ);
      printf("Size (in bytes) of buffer being used: %d...\n", GD_BUFLEN);
    }

    groupDescriptorBuffer = shared_ptr<char[]>(new char[GD_BUFLEN]);

    fs->seekg(DESC_TABLE_BLOCK * meta->blockSize, std::ios::beg);
    fs->read(groupDescriptorBuffer.get(), DESC_TABLE_SZ);
  }

  return groupDescriptorBuffer;
}

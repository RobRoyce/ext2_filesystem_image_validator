#include "imagereader.hpp"

ImageReader::ImageReader(char *filename) {

  this->metaFileName = std::string(filename);
  int rc = stat(filename, &this->metaFileStat);

  if (rc != 0)
    throw "Invalid File";

}

struct ext2_super_block &ImageReader::getSuperBlock()
{
  return this->superBlock;
}

size_t ImageReader::getBlockSize()
{
  return this->blockSize;
}

size_t ImageReader::getBlockGroupSize()
{
  return this->blockGroupSize;
}

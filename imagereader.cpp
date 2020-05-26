#include "imagereader.hpp"

ImageReader::ImageReader(MetaFile *metafile) 
{
  this->meta = metafile;
}

struct ext2_super_block *ImageReader::getSuperBlock()
{
  return &this->superBlock;
}

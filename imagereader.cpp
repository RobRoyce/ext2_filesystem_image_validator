// NAME: Rob Royce, Tyler Hackett
// EMAIL: robroyce1@ucla.edu, tjhackett@ucla.edu
// ID: 705357270, 405180956
#include "imagereader.hpp"

ImageReader::ImageReader(MetaFile *metafile) 
{
  this->meta = metafile;
}

struct ext2_super_block *ImageReader::getSuperBlock()
{
  return &this->superBlock;
}

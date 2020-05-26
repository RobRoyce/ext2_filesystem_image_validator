#include <sys/types.h>
#include "ext2_fs.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <string.h>
#include <sstream>
#include <sys/stat.h>
#include <vector>


#define KiB 1024
#define SUPERBLOCK_SIZE sizeof(ext2_super_block)
#define EXT2_OLD_REV 0
#define EXT2_DYNAMIC_REV 1

extern int debug;

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::runtime_error;

// -------------------------------------------------- Meta File Info
struct MetaFileLimits {
  // Limits enforced by block size
  // Based on Table 2.1 at
  // https://www.nongnu.org/ext2-doc/ext2.html#behaviour-flags
  unsigned int fileSystemBlocks;
  unsigned int blocksPerBlockGroup;
  unsigned int inodesPerBlockGroup;
  unsigned int bytesPerBlockGroup;
  unsigned long fileSystemSizeReal;
  unsigned long fileSystemSizeLinux;
  unsigned long blocksPerFile;
  unsigned long long fileSizeReal;
  unsigned long long fileSizeLinux;
};



/**
   MetaFile contains pertinent data fields regarding the actual EXT2 file
   system. There are 2 levels of meta here, each with their own fields:

   1 -- The actual file we are reading
     -- stat, filename
   2 -- The interpretation of that file
     -- else (not stat, filename)

     Level 1 can be established without reading the superBlock, whereas level 2
     fields require fields from the superBlock. Level 2 fields are essentially
     for convenience, since they can easily be retrieved from the superBlock or
     the meta stat. Note that level 2 fields come before level 1 fields, due to
     the fact that level 1 fields (especially the filename) are variable in
     length, and so belong at the end of the struct to ensure optimal memory
     footprint.
 */
struct MetaFile {
 public:
  // Level 2
  __u32 blockCount;
  __u32 blockSize;

  __u32 inodeSize;
  __u32 inodesPerGroup;

  __u32 blocksPerGroup;
  __u32 blockGroupsCount;
  __u32 blockGroupsPerDescTable;

  __u32 rev;

  unsigned long long blockGroupSize;

  struct stat stat;
  std::string filename;

  MetaFile() {}
  ~MetaFile() {}

private:
  MetaFileLimits limits;

};



// -------------------------------------------------- Prototypical Block
class MemoryBlock {
 public:
   MemoryBlock() {}
   MemoryBlock(std::vector<char> data) : data(data), size(data.size()) {}

 private:
  std::vector<char> data;
  __u32 size;
};


class MemoryBlockCache {
 public:
   MemoryBlockCache() {}
   MemoryBlockCache(size_t nblocks) {
     blocks.reserve(nblocks);
   }
   ~MemoryBlockCache() {}

 private:
  std::vector<MemoryBlock> blocks;

};

enum MemoryManagement {
  DMA, MMIO, HYBRID
};


// -------------------------------------------------- EXT2
class EXT2 {
 public:
  // Member Variables


  // Methods
  EXT2(char *);
  ~EXT2();
  bool readSuperBlock(); // validate and populate superBlock
  bool parseSuperBlock(); // validate and populate metaFile

  void printSuperBlock();
  void printGroupSummary();
  void printFreeBlockEntries();
  void printFreeInodeEntries();
  void printInodeSummary();
  void printDirectoryEntries();
  void printIndirectBlockRefs();


 private:
  // Member Variables
  std::unique_ptr<ext2_super_block> superBlock;
  std::unique_ptr<MetaFile> meta;
  std::unique_ptr<MemoryBlockCache> blocks;

  // Methods
  bool getMetaFileInfo();
  bool validateSuperBlock();
};

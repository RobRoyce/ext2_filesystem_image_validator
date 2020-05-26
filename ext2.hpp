#include <sys/types.h>
#include "imagereader.hpp"
#include "metafile.hpp"
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
using std::ifstream;
using std::make_unique;
using std::string;
using std::runtime_error;
using std::unique_ptr;
using std::vector;

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
  vector<MemoryBlock> blocks;

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
  void printDescTable(struct ext2_group_desc gd) {
    printf("Block Bitmap: %x...\n", gd.bg_block_bitmap);
    printf("Inode Bitmap: %x...\n", gd.bg_inode_bitmap);
    printf("Inode Table: %x...\n", gd.bg_inode_table);
    printf("Free Block Count: %x...\n", gd.bg_free_blocks_count);
    printf("Free Inodes Count: %x...\n", gd.bg_free_inodes_count);
    printf("Used Dirs Count: %x...\n", gd.bg_used_dirs_count);
    printf("Padding: %x...\n", gd.bg_pad);
    printf("Reserved Size: 0x%lx...\n", sizeof(gd.bg_reserved));
  }


 private:
  // Member Variables
  ImageReader *imReader;

  unique_ptr<vector<ext2_group_desc>> groupDescTbl;
  unique_ptr<MetaFile> meta = nullptr;

  // Methods

  void blockDump(size_t);

  bool getMetaFileInfo();
  bool getGroupDesc();
  bool validateSuperBlock();
};

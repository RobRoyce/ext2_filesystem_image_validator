#include "imagereader.hpp"
#include "metafile.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <ctime>

#define KiB 1024
#define SUPERBLOCK_SIZE sizeof(ext2_super_block)
#define EXT2_OLD_REV 0
#define EXT2_DYNAMIC_REV 1

const __u8 MASK = 0xFF;
const __u32 MASK_SIZE = sizeof(__u8) * 8;

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

// -------------------------------------------------- EXT2
class EXT2 {
 public:
  EXT2(char *);
  ~EXT2();
  bool readSuperBlock(); // validate and populate superBlock
  bool parseSuperBlock(); // validate and populate metaFile

  // Top Level Reporting Methods
  void printSuperBlock();
  void printGroupSummary();
  void printFreeBlockEntries();
  void printFreeInodeEntries();
  void printInodeSummary();
  void printDirectoryEntries();
  void printIndirectBlockRefs();

 private:
  unique_ptr<ImageReader> imReader = nullptr;
  unique_ptr<MetaFile> meta = nullptr;
  unique_ptr<vector<ext2_group_desc>> groupDescTbl;

  void blockDump(size_t);
  bool getMetaFileInfo();
  bool getGroupDescTbl();
  bool validateSuperBlock();
  void printDescTable(struct ext2_group_desc);
  int blocksInLastGroup();
};

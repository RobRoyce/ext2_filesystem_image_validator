#include <sys/types.h>
#include "ext2_fs.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string.h>
#include <sstream>
#include <sys/stat.h>
#include <vector>


#define KiB 1024
#define SUPERBLOCK_SIZE sizeof(ext2_super_block)

extern int debug;

using std::cout;
using std::cin;
using std::endl;
using std::string;


// -------------------------------------------------- EXT2 Class
class EXT2 {
 public:
  EXT2(char *);

 private:
  // Member Variables
  struct ext2_super_block *superBlock;
  struct ext2_dir_entry *ext2DirEntry;
  struct stat metaFileStat;
  std::string metaFileName;
  __u32 rev;

  // Methods
  int readSuperBlock();
  void printSuperBlock();
};

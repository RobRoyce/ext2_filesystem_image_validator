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

extern int debug;

using std::cout;
using std::cin;
using std::endl;
using std::string;


// -------------------------------------------------- EXT2 Class
class EXT2 {
 public:
   // Member Variables


   // Methods
   EXT2(char *);
  ~EXT2();
   int readSuperBlock();
   int parseSuperBlock();
   void printSuperBlock();

 private:
   // Member Variables
  std::shared_ptr<ext2_super_block> sblock;
  ext2_super_block *superBlock;
   // struct ext2_super_block *superBlock;
   struct stat metaFileStat;
   std::string metaFileName;
   __u32 rev;
  __u32 block_size;
  __u32 inode_size;
   // Methods
};

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

extern int debug;

using std::cout;
using std::cin;
using std::endl;
using std::string;

// -------------------------------------------------- Composite EXT2 Structure
struct SuperBlock {
  ext2_super_block sbCore;
  unsigned int blockSize;
  unsigned int totalSize;
};


// -------------------------------------------------- EXT2 Class
class EXT2 {
 public:
  EXT2(char *);
  unsigned int block_size();
 private:
  struct SuperBlock superBlock;
  struct stat metaFileStat;
  std::string metaFileName;
  int readSuperBlock();
};

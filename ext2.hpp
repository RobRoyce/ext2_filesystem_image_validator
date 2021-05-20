#include "imagereader.hpp"
#include "metafile.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdexcept>
#include <vector>
#include <ctime>

#define KiB 1024
#define SUPERBLOCK_SIZE sizeof(ext2_super_block)
#define EXT2_OLD_REV 0
#define EXT2_DYNAMIC_REV 1
#define EXT2_OLD_INODE_SIZE 128
#define IMPOSSIBLE_MALLOC "MemoryAllocationImpossible"

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
using std::list;

class EXT2_error : public std::exception {
  using std::exception::what;
public:
  explicit EXT2_error(const char *msg) : s_(msg){}
  explicit EXT2_error(const string &msg) : s_(msg) {}
  virtual ~EXT2_error () {}
  virtual const char* what() { return s_.c_str(); }
 protected:
  string s_;
};

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
  // void printDirectoryEntries();
  

 private:
  // ~imReader~ provides an interface for file operations
  unique_ptr<ImageReader> imReader = nullptr;

  // ~meta~ contains information about the file system that is not kept in the
  // file system itself
  unique_ptr<MetaFile> meta = nullptr;

  // ~groupDescTbl~ contains a copy of the /first/ Group Descriptor Table
  unique_ptr<vector<ext2_group_desc>> groupDescTbl = nullptr;


  unique_ptr<ext2_inode> rootInode = nullptr;
  unique_ptr<vector<ext2_inode>> inodeTbl = nullptr;
  unique_ptr<vector<ext2_inode *>> inodeDirType = nullptr;
  unique_ptr<list<ext2_dir_entry>> dirTree = nullptr;


  void blockDump(size_t);
  // void buildDirectoryTree(); // throws labeled exception
  bool setRevisionParameters();
  void getMetaFileInfo(ext2_super_block*);
  bool getGroupDescTbl();

  void printDirInode(ext2_inode*, size_t);
  void printIndirectBlockRefs(shared_ptr<char[]>, size_t, size_t, size_t, size_t);


  bool validateSuperBlock(); // throws labeled runtime_error
  void printDescTable(struct ext2_group_desc);
  void setBlocksInLastGroup();
  void setInodesInLastGroup();
};



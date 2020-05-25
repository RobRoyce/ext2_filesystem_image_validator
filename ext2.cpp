#include "ext2.hpp"
#include "bufferedimagereader.hpp"

#include <iomanip>


EXT2::EXT2(char *filename) {

  this->imReader = new BufferedImageReader(filename);
  this->imReader->init();

  printSuperBlock();
  blockDump(1);
}


void EXT2::printSuperBlock()
{
  struct ext2_super_block superBlock = this->imReader->getSuperBlock();

  cout << "inodes count: " << superBlock.s_inodes_count << endl;
  cout << "blocks count: " << superBlock.s_blocks_count << endl;
  cout << "reserved blocks count: " << superBlock.s_r_blocks_count << endl;
  cout << "free blocks count: " << superBlock.s_free_blocks_count << endl;
  cout << "free inodes count: " << superBlock.s_free_inodes_count << endl;
  cout << "first data block: " << superBlock.s_first_data_block << endl;
  cout << "log block size: " << superBlock.s_log_block_size << endl;
  cout << "log frag size: " << superBlock.s_log_frag_size << endl;
  cout << "blocks per group: " << superBlock.s_blocks_per_group << endl;
  cout << "frags per group: " << superBlock.s_frags_per_group << endl;
  cout << "inodes per group: " << superBlock.s_inodes_per_group << endl;
  cout << "mount time: " << superBlock.s_mtime << endl;
  cout << "write time: " << superBlock.s_wtime << endl;
  cout << "mount count: " << superBlock.s_mnt_count << endl;
  cout << "max mount count: " << superBlock.s_max_mnt_count << endl;
  cout << "magic signature: " << superBlock.s_magic << endl;
  cout << "file system state: " << superBlock.s_state << endl;
  cout << "errors: " << superBlock.s_errors << endl;
  cout << "minor revision level: " << superBlock.s_minor_rev_level << endl;
  cout << "time of last check: " << superBlock.s_lastcheck << endl;
  cout << "max time between checks: " << superBlock.s_checkinterval << endl;
  cout << "creator OS: " << superBlock.s_creator_os << endl;
  cout << "revision level: " << superBlock.s_rev_level << endl;
  cout << "default uid for reserved blocks: " << superBlock.s_def_resuid << endl;
  cout << "default gid for reserved blocks: " << superBlock.s_def_resgid << endl;
  cout << "first non-reserved inode: " << superBlock.s_first_ino << endl;
  cout << "size of inode structure: " << superBlock.s_inode_size << endl;
  cout << "compatible feature set: " << superBlock.s_feature_compat << endl;
  cout << "incompatible feature set: " << superBlock.s_feature_incompat << endl;
  cout << "readonly-compatible feature set: " << superBlock.s_feature_ro_compat << endl;
  cout << "Reserved padding (size: " << sizeof(superBlock.s_reserved)
       << "): " << superBlock.s_reserved << endl;
}

void EXT2::blockDump(size_t blockIdx)
{
  short *block = static_cast<short*>(this->imReader->getBlock(blockIdx));

  static const size_t num_cols = 8;

  for(size_t i = 0; i < this->imReader->getBlockSize()/num_cols; ++i)
  {
    std::cout << std::hex << std::setfill('0') << std::setw(4) << (i*num_rows) << ": ";
    
    for(size_t j = 0; j < num_rows; ++j)
    {
      std::cout << std::hex << std::setfill('0') << std::setw(4) << block[num_rows*i + j] << " ";
    }
    
    std::cout << std::endl;

  }
  
  std::cout << std::endl;

}

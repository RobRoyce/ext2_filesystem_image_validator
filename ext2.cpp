#include "ext2.hpp"


EXT2::EXT2(char *filename) {
  this->metaFileName = std::string(filename);
  int rc = stat(filename, &this->metaFileStat);
  if (rc != 0)
    throw "Invalid File";

  this->superBlock = new ext2_super_block;
}

EXT2::~EXT2() {
  delete this->superBlock;
}

int EXT2::readSuperBlock() {
  std::stringstream buffer;
  std::ifstream fs(this->metaFileName, std::ios::binary | std::ios::in);
  if (!fs)
    return -1;

  char buf[sizeof(ext2_super_block)];

  fs.unsetf(std::ios::skipws);
  fs.seekg(KiB, std::ios::beg);
  fs.read(buf, sizeof(ext2_super_block));
  memcpy(this->superBlock, buf, sizeof(ext2_super_block));

  return 0;
}

int EXT2::parseSuperBlock() {
  // -- There are various things we need to do based on contents of the Super Block
  // -- Revision determines algorithm for lookup (hash map vs linked list)
  // -- Revision 1:
  // --- variable inode sizes
  // --- extended attributesa
  // --- Sparse SuperBlocks

  this->rev = this->superBlock->s_rev_level;
  this->block_size = this->metaFileStat.st_size / this->superBlock->s_blocks_count;

  // TODO this field only exists in rev1 ext2, need to figure out how to compute
  // it manually instead
  this->inode_size = this->superBlock->s_inode_size;
  return 0;
}


void EXT2::printSuperBlock() {
  printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
         this->superBlock->s_blocks_count,
         this->superBlock->s_inodes_count,
         this->block_size,
         this->inode_size,
         this->superBlock->s_blocks_per_group,
         this->superBlock->s_inodes_per_group,
         this->superBlock->s_first_ino
         );

  // TODO: Remove everything below
  // cout << "inodes count: " << this->superBlock->s_inodes_count << endl;
  // cout << "blocks count: " << this->superBlock->s_blocks_count << endl;
  // cout << "reserved blocks count: " << this->superBlock->s_r_blocks_count << endl;
  // cout << "free blocks count: " << this->superBlock->s_free_blocks_count << endl;
  // cout << "free inodes count: " << this->superBlock->s_free_inodes_count << endl;
  // cout << "first data block: " << this->superBlock->s_first_data_block << endl;
  // cout << "log block size: " << this->superBlock->s_log_block_size << endl;
  // cout << "log frag size: " << this->superBlock->s_log_frag_size << endl;
  // cout << "blocks per group: " << this->superBlock->s_blocks_per_group << endl;
  // cout << "frags per group: " << this->superBlock->s_frags_per_group << endl;
  // cout << "inodes per group: " << this->superBlock->s_inodes_per_group << endl;
  // cout << "mount time: " << this->superBlock->s_mtime << endl;
  // cout << "write time: " << this->superBlock->s_wtime << endl;
  // cout << "mount count: " << this->superBlock->s_mnt_count << endl;
  // cout << "max mount count: " << this->superBlock->s_max_mnt_count << endl;
  // cout << "magic signature: " << this->superBlock->s_magic << endl;
  // cout << "file system state: " << this->superBlock->s_state << endl;
  // cout << "errors: " << this->superBlock->s_errors << endl;
  // cout << "minor revision level: " << this->superBlock->s_minor_rev_level << endl;
  // cout << "time of last check: " << this->superBlock->s_lastcheck << endl;
  // cout << "max time between checks: " << this->superBlock->s_checkinterval << endl;
  // cout << "creator OS: " << this->superBlock->s_creator_os << endl;
  // cout << "revision level: " << this->superBlock->s_rev_level << endl;
  // cout << "default uid for reserved blocks: " << this->superBlock->s_def_resuid << endl;
  // cout << "default gid for reserved blocks: " << this->superBlock->s_def_resgid << endl;
  // cout << "first non-reserved inode: " << this->superBlock->s_first_ino << endl;
  // cout << "size of inode structure: " << this->superBlock->s_inode_size << endl;
  // cout << "compatible feature set: " << this->superBlock->s_feature_compat << endl;
  // cout << "incompatible feature set: " << this->superBlock->s_feature_incompat << endl;
  // cout << "readonly-compatible feature set: " << this->superBlock->s_feature_ro_compat << endl;
  // cout << "Reserved padding (size: " << sizeof(this->superBlock->s_reserved)
  //      << "): " << this->superBlock->s_reserved << endl;
}

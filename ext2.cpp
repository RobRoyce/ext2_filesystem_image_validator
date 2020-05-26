#include "ext2.hpp"
#include "bufferedimagereader.hpp"
#include <iomanip>

EXT2::EXT2(char *filename) {

  // -------------------------------------------------- Initial Meta Check
  try {
    meta = std::make_unique<MetaFile>();
  } catch (...) {
    throw runtime_error("FileSystemAllocationError");
  }

  meta->filename = std::string(filename);

  if (stat(meta->filename.c_str(), &meta->stat) != 0)
    throw runtime_error("FileSystemReadError");

  // -------------------------------------------------- Init Reader and Super Block
  // slightly hacky way to use smart pointers with polymorphism (improvements welcomed)
  unique_ptr<ImageReader> base(new BufferedImageReader(meta.get()));
  imReader = std::move(base);
  imReader->init(); // Automatically initializes the super block

  if (!parseSuperBlock())
    throw runtime_error("SuperBlockParseError");

  // -------------------------------------------------- Integrity Check Meta
  if(!getMetaFileInfo())
    throw runtime_error("FileSystemInvalidError");

  if(!validateSuperBlock())
    throw runtime_error("SuperBlockInvalidError");

  // -------------------------------------------------- Populate Group Descriptor Table
  if(!getGroupDescTbl())
    throw runtime_error("GroupDescriptionReadError");
}

// --------------------------------------------------Destructor
EXT2::~EXT2() {}

bool EXT2::getGroupDescTbl() {
  // Descriptor Table is located at block 1 if block size is 1KiB, otherwise block 2
  const __u32 DESC_TABLE_LEN = meta->blockGroupsCount;

  char *buf = static_cast<char*>(this->imReader->getGroupDescriptor());

  // buf now contains the entire block group descriptor table
  // create group descriptor entires and append to the vector groupDescTbl
  groupDescTbl = make_unique<vector<ext2_group_desc>>();
  groupDescTbl->reserve(DESC_TABLE_LEN);

  for (__u32 i = 0; i < DESC_TABLE_LEN; i++) {
    unique_ptr<ext2_group_desc> tmp = make_unique<ext2_group_desc>();
    if(memcpy(tmp.get(), &buf[i], sizeof(ext2_group_desc)) == nullptr)
      return false;
    groupDescTbl->push_back(std::move(*tmp));
  }

  if (debug) {
    int i = 0;
    for(auto groupDesc : *groupDescTbl) {
      printf("-------------------------------------------------- Group Descriptor %d:\n", i++);
      printDescTable(groupDesc);
      printf("\n");
    }
  }

  return true;
}

bool EXT2::getMetaFileInfo() {
  // TODO implementation
  return true;
}

bool EXT2::parseSuperBlock() {
  // Attempt to perform any type of inferential or explicit validation using the
  // specifications given for EXT2. We're mostly looking for inconsistencies here.

  ext2_super_block *superBlock = this->imReader->getSuperBlock();

  // Block size can be calculated in two ways. First, it should be the total
  // file size divided by the number of blocks (file.size /
  // superBlock.blockCount) Second, it can be found explicitly using
  // (blockSize = 1024 << log_block_size) We can use this for validatiion
  // purposes.
  __u32 blockSize1 = meta->stat.st_size / superBlock->s_blocks_count;
  __u32 blockSize2 = KiB << superBlock->s_log_block_size;

  if(debug)
    printf("Blocksize Calculations: [1:%d] [2:%d]...\n", blockSize1, blockSize2);
  if (blockSize1 != blockSize2)
    return false;


  meta->blockCount = superBlock->s_blocks_count; // how many blocks in the file system?
  meta->blockSize = blockSize2; // what size is each block?
  meta->inodesPerGroup = superBlock->s_inodes_per_group; // How many inodes per group?
  meta->blocksPerGroup = superBlock->s_blocks_per_group; // how many blocks per group?
  meta->blockGroupSize = meta->blockSize * meta->blocksPerGroup; // what size is each block group?

  if (debug) {
    printf("Block Count: %d...\n", meta->blockCount);
    printf("Block Size: %d...\n", meta->blockSize);
    printf("Inodes Per Group: %d...\n", meta->inodesPerGroup);
    printf("Blocks Per Group: %d...\n", meta->blocksPerGroup);
    printf("Block Group Size: %lld...\n", meta->blockGroupSize);
  }

  // -- Note: the spec for project3a explicitly states that there will only be a single
  // -- block group, so we can make some assumptions about valid input
  // how many block groups are there?
  meta->blockGroupsCount = (meta->blockCount < meta->blocksPerGroup) ? 1 : meta->blockCount / meta->blocksPerGroup;

  if(debug)
    printf("Number of Block Groups: %d...\n", meta->blockGroupsCount);
  if (meta->blockGroupsCount != 1)
    return false;


  // -- There are various things we need to do based on contents of the Super
  // Block
  // -- Revision determines algorithm for lookup (hash map vs linked list)
  // -- Revision 1:
  // --- variable inode sizes
  // --- extended attributesa
  // --- Sparse SuperBlocks
  meta->rev = superBlock->s_rev_level;

  if(debug)
    printf("Revision Level: %d...\n", meta->rev);

  switch(meta->rev) {
    case EXT2_OLD_REV:

      break;
    case EXT2_DYNAMIC_REV:
      meta->inodeSize = superBlock->s_inode_size;

      break;
    default:
    break;
  }

  return true;
  }

void EXT2::printSuperBlock() {

  ext2_super_block *superBlock = this->imReader->getSuperBlock();

  printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
         superBlock->s_blocks_count,
         superBlock->s_inodes_count,
         meta->blockSize,
         meta->inodeSize,
         superBlock->s_blocks_per_group,
         superBlock->s_inodes_per_group,
         superBlock->s_first_ino
         );


  // // TODO: Remove everything below
  // if (debug) {
  //   cout << "inodes count: " << superBlock->s_inodes_count << endl;
  //   cout << "blocks count: " << superBlock->s_blocks_count << endl;
  //   cout << "reserved blocks count: " << superBlock->s_r_blocks_count << endl;
  //   cout << "free blocks count: " << superBlock->s_free_blocks_count << endl;
  //   cout << "free inodes count: " << superBlock->s_free_inodes_count << endl;
  //   cout << "first data block: " << superBlock->s_first_data_block << endl;
  //   cout << "log block size: " << superBlock->s_log_block_size << endl;
  //   cout << "log frag size: " << superBlock->s_log_frag_size << endl;
  //   cout << "blocks per group: " << superBlock->s_blocks_per_group << endl;
  //   cout << "frags per group: " << superBlock->s_frags_per_group << endl;
  //   cout << "inodes per group: " << superBlock->s_inodes_per_group << endl;
  //   cout << "mount time: " << superBlock->s_mtime << endl;
  //   cout << "write time: " << superBlock->s_wtime << endl;
  //   cout << "mount count: " << superBlock->s_mnt_count << endl;
  //   cout << "max mount count: " << superBlock->s_max_mnt_count << endl;
  //   cout << "magic signature: " << superBlock->s_magic << endl;
  //   cout << "file system state: " << superBlock->s_state << endl;
  //   cout << "errors: " << superBlock->s_errors << endl;
  //   cout << "minor revision level: " << superBlock->s_minor_rev_level << endl;
  //   cout << "time of last check: " << superBlock->s_lastcheck << endl;
  //   cout << "max time between checks: " << superBlock->s_checkinterval << endl;
  //   cout << "creator OS: " << superBlock->s_creator_os << endl;
  //   cout << "revision level: " << superBlock->s_rev_level << endl;
  //   cout << "default uid for reserved blocks: " << superBlock->s_def_resuid
  //        << endl;
  //   cout << "default gid for reserved blocks: " << superBlock->s_def_resgid
  //        << endl;
  //   cout << "first non-reserved inode: " << superBlock->s_first_ino << endl;
  //   cout << "size of inode structure: " << superBlock->s_inode_size << endl;
  //   cout << "compatible feature set: " << superBlock->s_feature_compat << endl;
  //   cout << "incompatible feature set: " << superBlock->s_feature_incompat
  //        << endl;
  //   cout << "readonly-compatible feature set: "
  //        << superBlock->s_feature_ro_compat << endl;
  //   cout << "Reserved padding (size: " << sizeof(superBlock->s_reserved)
  //        << "): " << superBlock->s_reserved << endl;
  // }
}

void EXT2::printGroupSummary() {
  const __u32 GROUP_COUNT = groupDescTbl->size();
  __u32 GS2 = 0; // group number
  __u32 GS3 = 0;
  __u32 GS4 = meta->inodesPerGroup;
  __u16 GS5 = 0;
  __u16 GS6 = 0;
  __u32 GS7 = 0;
  __u32 GS8 = 0;
  __u32 GS9 = 0;

  for (auto groupDesc : *groupDescTbl) {
    if (GS2 == GROUP_COUNT - 1)
      // Calculation required, since this might not be a full group
      // (file.size - bytes from full groups) / blocksize
      GS3 = (meta->stat.st_size - ((GROUP_COUNT - 1) * meta->blockGroupSize)) /
            meta->blockSize;
    else
      GS3 = meta->blocksPerGroup;

    GS5 = groupDesc.bg_free_blocks_count;
    GS6 = groupDesc.bg_free_inodes_count;
    GS7 = groupDesc.bg_block_bitmap;
    GS8 = groupDesc.bg_inode_bitmap;
    GS9 = groupDesc.bg_inode_table;

    printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", GS2++, GS3, GS4, GS5, GS6, GS7,
           GS8, GS9);
  }
}

void EXT2::printFreeBlockEntries(){}
void EXT2::printFreeInodeEntries(){}
void EXT2::printInodeSummary(){}
void EXT2::printDirectoryEntries(){}
void EXT2::printIndirectBlockRefs(){}


/*PRIVATE*/
bool EXT2::validateSuperBlock() {

  ext2_super_block *superBlock = this->imReader->getSuperBlock();

  // Magic Number Check
  // EXT2 uses the magic number 0xEF53
  if(superBlock->s_magic != EXT2_SUPER_MAGIC)
    return false;

  // The major revision number will inform the DS&A we use to read the FS. Here,
  // we check the revision number and set specific flags that we will use during
  // reading and parsing.
  __u32 rev = superBlock->s_rev_level;

  switch(rev) {
    case 0: // Revision 0
      if(superBlock->s_first_ino != EXT2_GOOD_OLD_FIRST_INO)
        return false;

      break;

    case 1: // Revision 1
      break;

    default: // Error
      return false;
  }

  return true;
}


/*PRIVATE*/
void EXT2::printDescTable(struct ext2_group_desc gd) {
  printf("Block Bitmap: %x...\n", gd.bg_block_bitmap);
  printf("Inode Bitmap: %x...\n", gd.bg_inode_bitmap);
  printf("Inode Table: %x...\n", gd.bg_inode_table);
  printf("Free Block Count: %x...\n", gd.bg_free_blocks_count);
  printf("Free Inodes Count: %x...\n", gd.bg_free_inodes_count);
  printf("Used Dirs Count: %x...\n", gd.bg_used_dirs_count);
  printf("Padding: %x...\n", gd.bg_pad);
  printf("Reserved Size: 0x%lx...\n", sizeof(gd.bg_reserved));
}

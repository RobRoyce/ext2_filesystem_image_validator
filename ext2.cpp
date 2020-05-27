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

  if (!parseSuperBlock())
    throw runtime_error("SuperBlockParseError");


  imReader->init();

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
    GS3 = (GS2 == GROUP_COUNT-1) ? blocksInLastGroup() : meta->blocksPerGroup;
    GS5 = groupDesc.bg_free_blocks_count;
    GS6 = groupDesc.bg_free_inodes_count;
    GS7 = groupDesc.bg_block_bitmap;
    GS8 = groupDesc.bg_inode_bitmap;
    GS9 = groupDesc.bg_inode_table;

    printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", GS2++, GS3, GS4, GS5, GS6, GS7,
           GS8, GS9);
  }
}

void EXT2::printFreeBlockEntries(){
  // Block 1 corresponds to bit 0 of byte 0
  const __u32 GROUP_COUNT = groupDescTbl->size();
  __u32 bitmapSize = meta->blocksPerGroup;
  __u32 iters = bitmapSize / MASK_SIZE;
  __u32 bitmapAddr = 0;
  __u32 index = 0;
  const __u8 residuals = bitmapSize % MASK_SIZE;
  iters = residuals ? iters + 1 : iters;


  for(auto groupDesc : *groupDescTbl) {
    bitmapAddr = groupDesc.bg_block_bitmap;
    char *buf = static_cast<char *>(imReader->getBlock(bitmapAddr));

    if(index++ == GROUP_COUNT - 1)
      bitmapSize = blocksInLastGroup();

    if (debug) {
      printf("-------------------------------------------------- printFreeBlockEntries()\n");
      printf("Group Count: %d...\n", GROUP_COUNT);
      printf("Bitmap Size: %d...\n", bitmapSize);
      printf("Bitmap Block Address: %d...\n", bitmapAddr);
      printf("Number of Iterations (8bpi): %d...\n", bitmapSize/MASK_SIZE);
      printf("-------------------------------------------------- /printFreeBlockEntries()\n");
    }


    __u8 bitMask = 0x00;
    for (__u32 maskIt = 0x00000000; maskIt < iters; maskIt += 0x00000001) {
      __u8 bit = 1;
      bitMask = buf[maskIt] & MASK;


      if(maskIt == iters - 1 && residuals) {
        // if the last byte contains padding
        for (__u32 k = 0; k < residuals; k++, bit <<= 1)
          if (!(bitMask & bit))
            printf("BFREE,%d\n", (maskIt * 8) + k + 1);
      } else {
        for (__u32 k = 0; k < MASK_SIZE; k++, bit <<= 1)
          if (!(bitMask & bit))
            printf("BFREE,%d\n", (maskIt * 8) + k + 1);
      }
    }
  }
}

void EXT2::printFreeInodeEntries(){
  __u32 bitmapSize = meta->inodesPerGroup;
  __u32 iters = bitmapSize / MASK_SIZE;
  __u32 bitmapAddr = 0;
  const __u8 residuals = bitmapSize % MASK_SIZE;
  iters = residuals ? iters + 1 : iters;

  for (auto groupDesc : *groupDescTbl) {
    bitmapAddr = groupDesc.bg_inode_bitmap;
    char *buf = static_cast<char *>(imReader->getBlock(bitmapAddr));

    if (debug) {
      printf("--------------------------------------------------printFreeInodeEntries()\n");
      printf("Bitmap Size: %d...\n", bitmapSize);
      printf("Bitmap Block Address: %d...\n", bitmapAddr);
      printf("Number of Iterations (8bpi): %d...\n", bitmapSize / MASK_SIZE);
      printf("--------------------------------------------------/printFreeInodekEntries()\n");
    }

    __u8 bitMask = 0x00;
    for(__u32 maskIt = 0x00000000; maskIt < iters; maskIt += 0x00000001) {
      __u8 bit = 1;
      bitMask = buf[maskIt] & MASK;

      if(maskIt == iters - 1 && residuals) {
        // if the last byte contains padding
        for(__u32 k = 0; k < residuals; k++, bit <<=1)
          if(!(bitMask & bit))
            printf("IFREE,%d\n", (maskIt * 8) + k + 1);
      } else {
        for (__u32 k = 0; k < MASK_SIZE; k++, bit <<= 1)
          if (!(bitMask & bit))
            printf("IFREE,%d\n", (maskIt * 8) + k + 1);
      }
    }
  }
}

void EXT2::printInodeSummary() {

  const unsigned INODE_TABLE_BLOCK_COUNT = (meta->inodesPerGroup / (meta->blockSize / meta->inodeSize));

  ext2_super_block *superBlock = imReader->getSuperBlock();

  for(auto groupDesc : *groupDescTbl) 
  {
    void *inodeBuffer = imReader->getBlocks(groupDesc.bg_inode_bitmap, 1 + INODE_TABLE_BLOCK_COUNT);

    char *inodeBitmap = static_cast<char*>(inodeBuffer); // the inode bitmap is the first block of the buffer
    ext2_inode *inodeTable = static_cast<ext2_inode*>(inodeBuffer + meta->blockSize); // inode table is 2nd block to end of inodeBuffer

    ext2_inode *currentInode;

    for(size_t i = 0; i < superBlock->s_inodes_count/8; ++i)
    {
      size_t inodeOffset = 8*i;

      for(size_t bitIdx = 0; bitIdx < 8; ++bitIdx)
      {
        if((inodeBitmap[i] >> bitIdx) & 0x01)
        {
          size_t inodeNumber = inodeOffset + bitIdx + 1; // Inode number starts at 1, not 0
          currentInode = &inodeTable[inodeNumber - 1]; 

          // Skip unallocated inodes
          if((currentInode->i_mode == 0) || (currentInode->i_links_count == 0))
            continue;

          char mode;

          // Time format: dd/mm/yy hh:mm:ss\0
          const size_t TIME_STR_LEN = 18;
          char cTimeStr[TIME_STR_LEN];
          char mTimeStr[TIME_STR_LEN];
          char aTimeStr[TIME_STR_LEN];

          time_t cTime = currentInode->i_ctime;
          time_t mTime = currentInode->i_mtime;
          time_t aTime = currentInode->i_atime;

          strftime(cTimeStr, TIME_STR_LEN, "%D %X", gmtime(&cTime));
          strftime(mTimeStr, TIME_STR_LEN, "%D %X", gmtime(&mTime));
          strftime(aTimeStr, TIME_STR_LEN, "%D %X", gmtime(&aTime));

          if(S_ISREG(currentInode->i_mode))
            mode = 'f';
          else if(S_ISDIR(currentInode->i_mode))
            mode = 'd';
          else if(S_ISLNK(currentInode->i_mode))
            mode = 's';
          else
            mode = '?';

          printf("INODE,%lu,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",
                inodeNumber,
                mode,
                currentInode->i_mode & 0x0FFF,
                currentInode->i_uid,
                currentInode->i_gid,
                currentInode->i_links_count,
                cTimeStr,
                mTimeStr,
                aTimeStr,
                currentInode->i_size,
                currentInode->i_blocks
                );

          if(((mode == 'f') || (mode == 'd')) || ((mode == 's' && currentInode->i_size > 60)))
          {
            for(size_t i = 0; i < 15; ++i)
            {
              printf(",%d", currentInode->i_block[i]);
            }
          }

          printf("\n");
        }
      }
    }
  }
}


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

int EXT2::blocksInLastGroup() {
  __u32 size = meta->stat.st_size;
  __u32 fullGroupsCount = groupDescTbl->size() - 1;
  __u32 groupSize = meta->blockGroupSize;
  __u32 blockSize = meta->blockSize;
  int res = (size - (fullGroupsCount * groupSize)) / blockSize;

  if(debug) {
    printf("-------------------------------------------------- blocksInLastGroup()\n");
    printf("File Size: %d...\n", size);
    printf("Number of Full Groups: %d...\n", fullGroupsCount);
    printf("Group Size: %d...\n", groupSize);
    printf("Block Size: %d...\n", blockSize);
    printf("Blocks in Last Group: %d...\n", res);
    printf("-------------------------------------------------- /blocksInLastGroup()\n");
  }

  return res;
}

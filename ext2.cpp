// NAME: Rob Royce, Tyler Hackett
// EMAIL: robroyce1@ucla.edu, tjhackett@ucla.edu
// ID: 705357270, 405180956
#include "ext2.hpp"
#include "bufferedimagereader.hpp"
#include <iomanip>

EXT2::EXT2(char *filename) {
  // -------------------------------------------------- Initial Meta Check
  try { meta = std::make_unique<MetaFile>(); }
  catch (...) { throw runtime_error("FileSystemAllocationError"); }

  meta->filename = string(filename);

  if (stat(meta->filename.c_str(), &meta->stat) != 0)
    throw runtime_error("FileSystemStatError");

  // -------------------------------------------------- Init Reader and Super Block
  // slightly hacky way to use smart pointers with polymorphism (improvements welcomed)
  unique_ptr<ImageReader> base(new BufferedImageReader(meta.get()));
  imReader = std::move(base);
  if(imReader == nullptr)
    throw EXT2_error("MemoryAllocationErrorDuringInitialFileSystemRead");


  try { parseSuperBlock(); }
  catch (EXT2_error &e) { throw e; }
  catch (...) { throw EXT2_error("SuperBlockParseError"); }

  imReader->init();

  // -------------------------------------------------- Super Block Validation
  try { validateSuperBlock(); }
  catch (runtime_error &e) { throw e; }
  catch (...) { throw runtime_error("SuperBlockValiationError"); }

  // -------------------------------------------------- Populate Group Descriptor Table
  try { getGroupDescTbl(); }
  catch (EXT2_error &e) { throw e; }
  catch (...) { throw EXT2_error("GroupDescriptorReadError"); }
}


EXT2::~EXT2() {}


bool EXT2::getGroupDescTbl() {
  // Descriptor Table is located at block 1 if block size is 1KiB, otherwise block 2
  const __u32 DESC_TABLE_LEN = meta->blockGroupsCount;
  char *buf;

  try {
    shared_ptr<char[]> bufPtr = this->imReader->getGroupDescriptor();
    buf = bufPtr.get();
  } catch (...) {
    throw EXT2_error("MalformeOrErroneousDescriptorTable");
  }


  // buf now contains the entire block group descriptor table
  // create group descriptor entires and append to the vector groupDescTbl
  try {
    groupDescTbl = make_unique<vector<ext2_group_desc>>();
    groupDescTbl->reserve(DESC_TABLE_LEN);
  }
  catch (...) { throw EXT2_error(IMPOSSIBLE_MALLOC); }

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

  // These can ONLY be done once the vector is initialized
  setBlocksInLastGroup();
  setInodesInLastGroup();

  return true;
}

void EXT2::getMetaFileInfo(ext2_super_block *sb) {
  // There are various things we need to do based on contents of the SuperBlock
  // Revision determines algorithm for lookup (hash map vs linked list)
  // - Revision 0:
  // - - 
  // - Revision 1:
  // - - variable inode sizes
  // - - extended attributesa
  // - - Sparse SuperBlocks
  meta->rev = sb->s_rev_level;
  meta->revMinor = sb->s_minor_rev_level;

  // Block size can be calculated in two ways. First, it should be the total
  // file size divided by the number of blocks
  // - - (file.size / superBlock.blockCount)
  //
  // Second, it can be found explicitly using
  // - - (blockSize = 1024 << s_log_block_size)
  //
  // We can use this for validation purposes.
  if(debug) {
    printf("File Size: %ld...\n", meta->stat.st_size);
    printf("Blocks Count: %d...\n", sb->s_blocks_count);
    printf("Block size: %d...\n", KiB << sb->s_log_block_size);
  }

  __u32 blockSize1 = meta->stat.st_size / sb->s_blocks_count;
  __u32 blockSize2 = KiB << sb->s_log_block_size;
  if (debug)
    printf("Blocksize Calculations: [1:%d] [2:%d]...\n", blockSize1, blockSize2);
  if (blockSize1 != blockSize2)
    throw EXT2_error("FileSystemMalformedBlockSizeError");

  // Set meta fields based on superBlock
  meta->blockCount = sb->s_blocks_count; // how many blocks in the file system?
  meta->blockSize = blockSize2;   // what size is each block?
  meta->inodesPerGroup = sb->s_inodes_per_group; // How many inodes per group?
  meta->blocksPerGroup = sb->s_blocks_per_group; // how many blocks per group?
  meta->blockGroupSize = meta->blockSize * meta->blocksPerGroup; // what size is each block group?

  // how many block groups are there? Note: the spec for project3a explicitly
  // states that there will only be a single block group, so we can make some
  // assumptions about valid input
  const __u32 bgc = meta->blockCount / meta->blocksPerGroup;
  meta->blockGroupsCount = (meta->blockCount < meta->blocksPerGroup) ? 1 : bgc;
  if (debug)
    printf("Number of Block Groups: %d...\n", meta->blockGroupsCount);
  if (meta->blockGroupsCount != 1)
    throw EXT2_error(
        "CS111ProjectSpecificationError -- Hi, TA... you promised there would "
        "only be one group max! Why have you done this to us? Surely you will "
        "not mark us down for this... considering it was in the specification :)");


  // --------------------------------------------------
  // Show me the money baby
  if (debug) {
    printf("Block Count: %d...\n", meta->blockCount);
    printf("Block Size: %d...\n", meta->blockSize);
    printf("Inodes Per Group: %d...\n", meta->inodesPerGroup);
    printf("Blocks Per Group: %d...\n", meta->blocksPerGroup);
    printf("Block Group Size: %lld...\n", meta->blockGroupSize);
  }




}

bool EXT2::parseSuperBlock() {
  // Attempt to perform any type of inferential or explicit validation using the
  // specifications given for EXT2. We're mostly looking for inconsistencies here.
  ext2_super_block *superBlock = imReader->getSuperBlock();


  // Magic Number Check
  // EXT2 uses the magic number 0xEF53
  if (superBlock->s_magic != EXT2_SUPER_MAGIC)
    throw EXT2_error("InvalidSuperBlockMagicNumber");

  try {
    getMetaFileInfo(superBlock);
  } catch(EXT2_error &e) {
    throw e;
  } catch(...) {
    throw "FileSystemMetaCorruptionDetected";
  }



  // -- There are various things we need to do based on contents of the Super Block
  // -- Revision determines algorithm for lookup (hash map vs linked list)
  // -- Revision 1:
  // --- variable inode sizes
  // --- extended attributesa
  // --- Sparse SuperBlocks
  if(debug)
    printf("Revision Level: %d...\n", meta->rev);

  switch(meta->rev) {
    case EXT2_OLD_REV:
      meta->inodeSize = EXT2_OLD_INODE_SIZE;
      break;
    case EXT2_DYNAMIC_REV:
      meta->inodeSize = superBlock->s_inode_size;
      break;
    default:
      break;
  }

  if(debug)
    printf("inode size: %d...\n", meta->inodeSize);

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
  if (groupDescTbl->size() <= 0)
    throw EXT2_error("EmptyGroupDescriptorTable");

  const __u32 GROUP_COUNT = groupDescTbl->size();
  __u32 GS2 = 0; // group number
  __u32 GS3 = 0; // blocks in group
  __u32 GS4 = meta->inodesPerGroup; // inodes per group
  __u16 GS5 = 0; // free blocks count
  __u16 GS6 = 0; // free inodes count
  __u32 GS7 = 0; // block bitmap
  __u32 GS8 = 0; // inode bitmap
  __u32 GS9 = 0; // inode table

  for (auto groupDesc : *groupDescTbl) {
    GS3 = (GS2 == GROUP_COUNT-1) ? meta->blocksInLastGroup : meta->blocksPerGroup;
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
  if (groupDescTbl->size() <= 0)
    throw EXT2_error("EmptyGroupDescriptorTable");

  const __u32 GROUP_COUNT = groupDescTbl->size();
  __u32 bitmapSize = meta->blocksPerGroup;
  __u32 iters = bitmapSize / MASK_SIZE;
  __u32 bitmapAddr = 0;
  __u32 index = 0;
  const __u8 residuals = bitmapSize % MASK_SIZE;

  // if (for some reason) the bitmap size is not a multiple of 8, then the final
  // byte will contain at least one bit which does not correspond to an actual
  // block entry. In this case, the value stored in 'iters' will have been
  // truncated. To compensate for this, we check for residuals (anything left
  // after the calculation of 'iters'). If so, that means we will need an
  // additional iteration to deal with those extra bits.
  iters = residuals ? iters + 1 : iters;

  // Block 1 corresponds to bit 0 of byte 0
  for(auto groupDesc : *groupDescTbl) {
    bitmapAddr = groupDesc.bg_block_bitmap;

    shared_ptr<char[]> bufPtr = imReader->getBlock(bitmapAddr);
    char *buf = bufPtr.get();

    if(index++ == GROUP_COUNT - 1)
      bitmapSize = meta->blocksInLastGroup;

    if (debug) {
      printf("-------------------------------------------------- printFreeBlockEntries()\n");
      printf("Group Count: %d...\n", GROUP_COUNT);
      printf("Bitmap Size: %d bits...\n", bitmapSize);
      printf("Bitmap Block Address: %d...\n", bitmapAddr);
      printf("Number of Iterations (8 bits per): %d...\n", bitmapSize/MASK_SIZE);
      printf("-------------------------------------------------- /printFreeBlockEntries()\n");
    }

    // perform bitmask then switch on the results (to print or not to print BFREE)
    __u8 bitMask = 0x00;
    for (__u32 maskIt = 0x00000000; maskIt < iters; maskIt += 0x00000001) {
      bitMask = buf[maskIt] & MASK;
      __u8 bit = 1;

      if(maskIt == iters - 1 && residuals) {
        // if the most significant byte contains padding
        for (__u32 k = 0; k < residuals; k++, bit <<= 1)
          if (!(bitMask & bit))
            printf("BFREE,%d\n", (maskIt * 8) + k + 1);

      } else {
        for (__u32 k = 0; k < MASK_SIZE; k++, bit <<= 1)
          if (!(bitMask & bit))
            printf("BFREE,%d\n", (maskIt * 8) + k + 1);
      } // END if(maskIt == iters...) else...

    } // END for(__u32 maskIt = 0x....)
  } // END for(auto groupDesc...)
}


void EXT2::printFreeInodeEntries(){
  __u32 bitmapSize = meta->inodesPerGroup;
  // TODO: will the bitmap size ALWAYS equal the number of inodes per group?

  __u32 iters = bitmapSize / MASK_SIZE;
  __u32 bitmapAddr = 0;
  const __u8 residuals = bitmapSize % MASK_SIZE;

  // if (for some reason) the bitmap size is not a multiple of 8, then the final
  // byte will contain at least one bit which does not correspond to an actual
  // inode entry. In this case, the value stored in 'iters' will have been
  // truncated. To compensate for this, we check for residuals (anything left
  // after the calculation of 'iters'). If so, that means we will need an
  // additional iteration to deal with those extra bits.
  iters = residuals ? iters + 1 : iters;

  for (auto groupDesc : *groupDescTbl) {
    bitmapAddr = groupDesc.bg_inode_bitmap;

    shared_ptr<char[]> bufPtr = imReader->getBlock(bitmapAddr);
    char *buf = bufPtr.get();

    if (debug) {
      printf("--------------------------------------------------printFreeInodeEntries()\n");
      printf("Bitmap Size: %d bits...\n", bitmapSize);
      printf("Bitmap Block Address: %d...\n", bitmapAddr);
      printf("Number of Iterations (8bpi): %d...\n", bitmapSize / MASK_SIZE);
      printf("--------------------------------------------------/printFreeInodeEntries()\n");
    }

    __u8 bitMask = 0x00;
    for(__u32 maskIt = 0x00000000; maskIt < iters; maskIt += 0x00000001) {
      bitMask = buf[maskIt] & MASK;
      __u8 bit = 1;

      if(maskIt == iters - 1 && residuals) {
        // if the last byte contains padding
        for(__u32 k = 0; k < residuals; k++, bit <<=1)
          if(!(bitMask & bit))
            printf("IFREE,%d\n", (maskIt * 8) + k + 1);

      } else {
        for (__u32 k = 0; k < MASK_SIZE; k++, bit <<= 1)
          if (!(bitMask & bit))
            printf("IFREE,%d\n", (maskIt * 8) + k + 1);
      } // END if(maskIt == iters...) else...

    }// END for(__u32 maskIt...)
  } // END for(auto groupDesc...)
}


void EXT2::printInodeSummary() {
  const unsigned INODE_TABLE_BLOCK_COUNT =
      (meta->inodesPerGroup /
       (meta->blockSize / meta->inodeSize));
  ext2_super_block *superBlock = imReader->getSuperBlock();

  for(auto groupDesc : *groupDescTbl) 
  {
    shared_ptr<char[]> inodeBufferPtr = imReader->getBlocks(groupDesc.bg_inode_bitmap, 1 + INODE_TABLE_BLOCK_COUNT);

    void *inodeBuffer = inodeBufferPtr.get();

    char *inodeBitmap = static_cast<char *>(
        inodeBuffer); // the inode bitmap is the first block of the buffer

    ext2_inode *inodeTable =
        static_cast<ext2_inode *>(inodeBuffer) +
        meta->blockSize / sizeof(ext2_inode); // inode table is 2nd block to end of inodeBuffer

    ext2_inode *currentInode;

    for(size_t i = 0; i < superBlock->s_inodes_count/8; ++i)
    {
      size_t inodeOffset = 8*i;

      for(size_t bitIdx = 0; bitIdx < 8; ++bitIdx)
      {
        if((inodeBitmap[i] >> bitIdx) & 0x01)
        {
          size_t inodeNumber = inodeOffset + bitIdx + 1; // Inode number starts at 1, not 0
          currentInode = reinterpret_cast<ext2_inode*>(((char*)inodeTable) + meta->inodeSize*(inodeNumber - 1)); 

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

          // Print out all of the directory entries
          if(mode == 'd') {
            printDirInode(currentInode, inodeNumber);
          }

          if(mode == 'd' || mode == 'f') {
            if(currentInode->i_block[EXT2_IND_BLOCK] != 0)
            {
              printIndirectBlockRefs(imReader->getBlock(currentInode->i_block[EXT2_IND_BLOCK], ImageReader::BlockPersistenceType::SHARED), 
                                     currentInode->i_block[EXT2_IND_BLOCK], 0, inodeNumber, 1);
            }
            if(currentInode->i_block[EXT2_DIND_BLOCK] != 0)
            {
              printIndirectBlockRefs(imReader->getBlock(currentInode->i_block[EXT2_DIND_BLOCK], ImageReader::BlockPersistenceType::SHARED), 
                                     currentInode->i_block[EXT2_DIND_BLOCK], 256, inodeNumber, 2);
            }
            if(currentInode->i_block[EXT2_TIND_BLOCK] != 0)
            {
              printIndirectBlockRefs(imReader->getBlock(currentInode->i_block[EXT2_TIND_BLOCK], ImageReader::BlockPersistenceType::SHARED), 
                                     currentInode->i_block[EXT2_TIND_BLOCK], 257*256, inodeNumber, 3);
            }
          }

        }
      }
    }
  }
}

void EXT2::printDirInode(ext2_inode *dirInode, size_t inodeNumber) {

  shared_ptr<char[]> dirBlock;
  struct ext2_dir_entry *entry;
  size_t entryOffset = 0;
  size_t logicalOffset = 0;

  //Scan the direct block references.
  for(int blockIdx = 0; blockIdx < 12; blockIdx++)
  {

    dirBlock = imReader->getBlock(dirInode->i_block[blockIdx]);
    entry = reinterpret_cast<ext2_dir_entry*>(dirBlock.get());

    while(entryOffset < dirInode->i_size)
    {
      if(entry->rec_len == 0)
        break;

      if(entry->inode != 0)
      {
        printf("DIRENT,%lu,%lu,%d,%d,%d,'%.*s'\n",
                inodeNumber,
                logicalOffset,
                entry->inode,
                entry->rec_len,
                entry->name_len,
                entry->name_len,
                entry->name);

        logicalOffset += entry->rec_len;
      }

      entry = reinterpret_cast<ext2_dir_entry*>((char*)entry + entry->rec_len);
      entryOffset += entry->rec_len;
    }
  }

  //Scan the singly indirect block.
  shared_ptr<char[]> sindBlock = imReader->getBlock(dirInode->i_block[EXT2_IND_BLOCK], ImageReader::BlockPersistenceType::SHARED);
  int *sindBlockEntries = reinterpret_cast<int*>(sindBlock.get());
  for(size_t iBlockIdx = 0; iBlockIdx < meta->blockSize/4; iBlockIdx++)
  {
    if(sindBlockEntries[iBlockIdx] == 0)
      continue;

    dirBlock = imReader->getBlock(sindBlockEntries[iBlockIdx]);
    entry = reinterpret_cast<ext2_dir_entry*>(dirBlock.get());

    while(entryOffset < dirInode->i_size)
    {
      if(entry->rec_len == 0)
        break;

      if(entry->inode != 0)
      {
        printf("DIRENT,%lu,%lu,%d,%d,%d,'%.*s'\n",
                inodeNumber,
                logicalOffset,
                entry->inode,
                entry->rec_len,
                entry->name_len,
                entry->name_len,
                entry->name);

        logicalOffset += entry->rec_len;
      }

      entry = reinterpret_cast<ext2_dir_entry*>((char*)entry + entry->rec_len);
      entryOffset += entry->rec_len;
    }
  }

  //Scan the doubly indirect block.
  shared_ptr<char[]> dindBlock = imReader->getBlock(dirInode->i_block[EXT2_DIND_BLOCK], ImageReader::BlockPersistenceType::SHARED);
  int *dindBlockEntries = reinterpret_cast<int*>(dindBlock.get());
  for(size_t diBlockIdx = 0; diBlockIdx < meta->blockSize/4; diBlockIdx++)
  {
    if(dindBlockEntries[diBlockIdx] == 0)
      continue;

    sindBlock = imReader->getBlock(dindBlockEntries[diBlockIdx], ImageReader::BlockPersistenceType::SHARED);
    sindBlockEntries = reinterpret_cast<int*>(sindBlock.get());

    for(size_t iBlockIdx = 0; iBlockIdx < meta->blockSize/4; iBlockIdx++)
    {
      if(sindBlockEntries[iBlockIdx] == 0)
        continue;

      dirBlock = imReader->getBlock(sindBlockEntries[iBlockIdx]);
      entry = reinterpret_cast<ext2_dir_entry*>(dirBlock.get());

      while(entryOffset < dirInode->i_size)
      {
        if(entry->rec_len == 0)
          break;

        if(entry->inode != 0)
        {
          printf("DIRENT,%lu,%lu,%d,%d,%d,'%.*s'\n",
                  inodeNumber,
                  logicalOffset,
                  entry->inode,
                  entry->rec_len,
                  entry->name_len,
                  entry->name_len,
                  entry->name);

          logicalOffset += entry->rec_len;
        }

        entry = reinterpret_cast<ext2_dir_entry*>((char*)entry + entry->rec_len);
        entryOffset += entry->rec_len;
      }
    }
  }

  //Scan the triply indirect block.
  shared_ptr<char[]> tindBlock = imReader->getBlock(dirInode->i_block[EXT2_TIND_BLOCK], ImageReader::BlockPersistenceType::SHARED);
  int *tindBlockEntries = reinterpret_cast<int*>(tindBlock.get());
  for(size_t tiBlockIdx = 0; tiBlockIdx < meta->blockSize/4; tiBlockIdx++)
  {
    if(tindBlockEntries[tiBlockIdx] == 0)
      continue;

    dindBlock = imReader->getBlock(tindBlockEntries[tiBlockIdx], ImageReader::BlockPersistenceType::SHARED);
    dindBlockEntries = reinterpret_cast<int*>(dindBlock.get());

    for(size_t diBlockIdx = 0; diBlockIdx < meta->blockSize/4; diBlockIdx++)
    {
      if(dindBlockEntries[diBlockIdx] == 0)
        continue;

      sindBlock = imReader->getBlock(dindBlockEntries[diBlockIdx], ImageReader::BlockPersistenceType::SHARED);
      sindBlockEntries = reinterpret_cast<int*>(sindBlock.get());

      for(size_t iBlockIdx = 0; iBlockIdx < meta->blockSize/4; iBlockIdx++)
      {
        if(sindBlockEntries[iBlockIdx] == 0)
          continue;

        dirBlock = imReader->getBlock(sindBlockEntries[iBlockIdx]);
        entry = reinterpret_cast<ext2_dir_entry*>(dirBlock.get());

        while(entryOffset < dirInode->i_size)
        {
          if(entry->rec_len == 0)
            break;

          if(entry->inode != 0)
          {
            printf("DIRENT,%lu,%lu,%d,%d,%d,'%.*s'\n",
                    inodeNumber,
                    logicalOffset,
                    entry->inode,
                    entry->rec_len,
                    entry->name_len,
                    entry->name_len,
                    entry->name);

            logicalOffset += entry->rec_len;
          }

          entry = reinterpret_cast<ext2_dir_entry*>((char*)entry + entry->rec_len);
          entryOffset += entry->rec_len;
        }
      }
    }
  }
}

void EXT2::printIndirectBlockRefs(shared_ptr<char[]> indBlock, size_t indBlockNum, size_t baseLogicalOffset, size_t inodeNum, size_t level)
{
  uint32_t *blockIdx = reinterpret_cast<uint32_t*>(indBlock.get());

  for(size_t i = 0; i < meta->blockSize/4; i++)
  {
    if(blockIdx[i] != 0)
    {
      printf("INDIRECT,%lu,%lu,%lu,%lu,%d\n",
            inodeNum,
            level,
            EXT2_NDIR_BLOCKS + baseLogicalOffset + i,
            indBlockNum,
            blockIdx[i]
            );

      if(level > 1)
        printIndirectBlockRefs(imReader->getBlock(blockIdx[i], ImageReader::BlockPersistenceType::SHARED), 
                               blockIdx[i], baseLogicalOffset, inodeNum, level - 1);
    }
  }
}

/*PRIVATE -- throws labeled runtime_error*/
bool EXT2::validateSuperBlock() {
  // Returns true if valid, else false. 
  ext2_super_block *superBlock = this->imReader->getSuperBlock();

  // The major revision number will inform the DS&A we use to read the FS. Here,
  // we check the revision number and set specific flags that we will use during
  // reading and parsing.
  meta->rev = superBlock->s_rev_level;

  switch(meta->rev) {
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
  printf("Block Bitmap: %d...\n", gd.bg_block_bitmap);
  printf("Inode Bitmap: %d...\n", gd.bg_inode_bitmap);
  printf("Inode Table: %d...\n", gd.bg_inode_table);
  printf("Free Block Count: %d...\n", gd.bg_free_blocks_count);
  printf("Free Inodes Count: %d...\n", gd.bg_free_inodes_count);
  printf("Used Dirs Count: %d...\n", gd.bg_used_dirs_count);
  printf("Padding: %d...\n", gd.bg_pad);
  printf("Reserved Size: 0x%lx...\n", sizeof(gd.bg_reserved));
}

void EXT2::setBlocksInLastGroup() {
  // --------------------------------------------------
  // Blocks in last group
  __u32 fsSize = meta->stat.st_size;
  __u32 blockSize = meta->blockSize;
  __u32 groupSize = meta->blockGroupSize;

  if(fsSize < groupSize || fsSize % groupSize != 0) {
    // The total number of blocks in the last group may not be equal to the
    // s_blocks_per_group value found in the Super Block.
    //
    // This might happen if:
    // A) (fs.size < group.size)
    // B) (fs.size % group.size) != 0
    //
    // If that is the case, we can find the total number of blocks in the last
    // group by finding the residual.
    if(groupDescTbl->size() <= 0)
      throw EXT2_error("EmptyGroupDescriptorTable");

    __u32 fullGroupsCount = groupDescTbl->size() - 1; // last group not included
    int res = (fsSize - (fullGroupsCount * groupSize)) / blockSize;

    if (debug) {
      printf("------------------------------blocksInLastGroup()\n");
      printf("File Size: %d...\n", fsSize);
      printf("Number of Full Groups: %d...\n", fullGroupsCount);
      printf("Group Size: %d...\n", groupSize);
      printf("Block Size: %d...\n", blockSize);
      printf("Blocks in Last Group: %d...\n", res);
      printf("------------------------------/blocksInLastGroup()\n");
    }

    meta->blocksInLastGroup = res;
  } else
    meta->blocksInLastGroup = meta->blocksPerGroup;
}

void EXT2::setInodesInLastGroup() {
  // --------------------------------------------------
  // Inodes in last group
  // TODO: verify that this can happen at all...
}


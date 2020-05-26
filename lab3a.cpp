// NAME: Rob Royce, Tyler Hackett
// EMAIL: robroyce1@ucla.edu, tjhackett@ucla.edu
// ID: 705357270

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "ext2.hpp"
#include "utils.h"
#include <sys/stat.h>

#define LAB3B_USAGE "Usage: lab3b FILE"


int debug = 1;
bool mem_mapped = false;


int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << LAB3B_USAGE << std::endl;
    std::cout << "lab3a: please specify a file system image." << std::endl;
    exit(1); // TODO proper exit code
  }


  // -------------------------------------------------- Check/Read FS
  std::unique_ptr<EXT2> ext2 = nullptr;

  try {
    ext2 = std::make_unique<EXT2>(argv[1]);
  } catch (runtime_error &e) {
    cout << LAB3B_USAGE << endl;
    cout << "lab3a: Exception Occurred -- " << e.what() << endl;
    exit(1); // TODO proper exit code
  }



  // -------------------------------------------------- Generate Reports
  ext2->printSuperBlock(); // DONE
  ext2->printGroupSummary(); // DONE
  ext2->printFreeBlockEntries();
  ext2->printFreeInodeEntries();
  ext2->printInodeSummary();
  ext2->printDirectoryEntries();
  ext2->printIndirectBlockRefs();

  return 0;
}

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "ext2.hpp"
#include <sys/stat.h>

#define LAB3B_USAGE "Usage: lab3a FILE"
#define ERR_INIT "lab3a: Exception occurred during initialization -- "
#define ERR_RUNTIME "lab3a: Exception occurred during run time -- "
#define EXSUCCESS 0
#define EXBADARG 1
#define EXCORRUPT 2

int debug = 0;

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << LAB3B_USAGE << std::endl;
    std::cerr << "lab3a: expected 1 argument, received %d. See usage example.\n";
    std::cerr.flush();
    exit(EXBADARG); // TODO proper exit code
  }


  // -------------------------------------------------- Check/Read FS
  std::unique_ptr<EXT2> ext2 = nullptr;

  try {
    ext2 = std::make_unique<EXT2>(argv[1]);
  } catch (EXT2_error &e) {
    // All errors that may occur during initialization will be treated
    // as "corruption" errors
    std::cerr << LAB3B_USAGE << endl;
    std::cerr << ERR_INIT  << e.what();
    std::cerr.flush();
    exit(EXCORRUPT);
  } catch (...) {
    std::cerr << LAB3B_USAGE << endl;
    std::cerr << "lab3a: encountered invalid or unsupported file" << endl;
    std::cerr.flush();
    exit(EXCORRUPT);
  }



  // -------------------------------------------------- Generate Reports
  try {
    ext2->printSuperBlock();
    ext2->printGroupSummary();
    ext2->printFreeBlockEntries();
    ext2->printFreeInodeEntries();
    ext2->printInodeSummary();
  } catch (runtime_error &e) {
    std::cerr << ERR_RUNTIME << e.what() << endl;
    std::cerr.flush();
    exit(EXCORRUPT);
  }
  return EXSUCCESS;
}

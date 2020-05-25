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

#define LAB3B_USAGE "Usage: lab3b file system"


int debug = 1;
bool mem_mapped = false;


int main(int argc, char **argv) {
  // -------------------------------------------------- Arg Parse Errors
  if (argc != 2) {
    std::cout << "lab3a: please specify a file system image." << std::endl;
    std::cout << LAB3B_USAGE << std::endl;
    exit(1); // TODO proper exit code
  }

  EXT2 *ext2;

  // -------------------------------------------------- Check/Read FS
  try {
    ext2 = new EXT2(argv[1]);
  } catch (...) {
    std::cout << "lab3a: please specify a file system image." << std::endl;
    std::cout << LAB3B_USAGE << std::endl;
    exit(1); // TODO proper exit code
  }

  // -------------------------------------------------- Read/Parse FS
  ext2->readSuperBlock();
  ext2->parseSuperBlock();

  if(debug)
    ext2->printSuperBlock();

  // -------------------------------------------------- Generate Reports

  delete ext2;
  return 0;
}

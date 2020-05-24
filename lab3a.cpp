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
#define MAX_SIZE_MM 10485760 // 10M

int debug = 1;
bool mem_mapped = false;


int main(int argc, char **argv) {
  // -------------------------------------------------- Arg Parse Errors
  if (argc != 2) {
    std::cout << "lab3a: please specify a file system image." << std::endl;
    std::cout << LAB3B_USAGE << std::endl;
    exit(1); // TODO proper exit code
  }


  // -------------------------------------------------- Check/Read FS
  EXT2 ext2(argv[1]);


  return 0;
}

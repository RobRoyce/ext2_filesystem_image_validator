#include "ext2.hpp"


EXT2::EXT2(char *filename) {
  this->metaFileName = std::string(filename);
  int rc = stat(filename, &this->metaFileStat);
  if (rc != 0)
    throw "Invalid File";


  this->readSuperBlock();
}

int EXT2::readSuperBlock() {
  std::stringstream buffer;
  std::ifstream fs(this->metaFileName, std::ios::binary | std::ios::in);
  if (!fs)
    return -1;

  int rc = 0;
  char buf[KiB];

  fs.unsetf(std::ios::skipws);
  fs.seekg(KiB, std::ios::beg);
  fs.read(buf, KiB);


  // TODO remove this
  if(debug) {
    cout << "Raw contents of Super Block (size: " << strlen(buf) << "):" << endl;
    for (int i = 0; i < KiB; i++)
      cout << buf[i];
    cout << endl;
  }



  return rc;
}

unsigned int EXT2::block_size() { return this->superBlock.blockSize; }

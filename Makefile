#NAME: Robert Ristine, Tyler Hackett
#EMAIL: robroyce1@ucla.edu, tjhackett@ucla.edu
#ID: 705357270,405180956
.PHONY: mount clean dist

CC = g++
CFLAGS = -Wall -Wextra -std=gnu++17
DEPENDENCIES.C = ext2.cpp imagereader.cpp bufferedimagereader.cpp
DIST = lab3a-705357270.tar.gz
MAIN.C = lab3a.cpp
MOUNT = fs
FILES = README bufferedimagereader.cpp bufferedimagereader.hpp ext2.cpp ext2.hpp ext2_fs.h imagereader.hpp imagereader.cpp lab3a.cpp Makefile metafile.hpp
EXEC = lab3a
LIBS = -static-libstdc++

CFLAGS = -Wall -Wextra
DFLAGS = -g

default: lab3a

clean:
	rm -f $(EXEC) $(DIST)

check: debug
	bash mkfs.sh
	cat ./log.out | grep -A 5 -B 5 -E "(DIR|SUPER|GROUP|lab3a)"

debug: $(MAIN.C)
	$(CC) $(CFLAGS) -g $(MAIN.C) $(DEPENDENCIES.C) -o $(EXEC) $(LIBS)

dist:
	tar -czvf $(DIST) $(FILES)

lab3a: $(MAIN.C)
	$(CC) $(CFLAGS) $(MAIN.C) $(DEPENDENCIES.C) -o $(EXEC) $(LIBS)

test: dist
	cp lab3a-705357270.tar.gz test/ && cd test && bash P3A_check.sh 705357270

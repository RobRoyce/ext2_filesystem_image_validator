#NAME: Robert Ristine, Tyler Hackett
#EMAIL: robroyce1@ucla.edu, tjhackett@ucla.edu
#ID: 705357270,

.PHONY: mount clean dist

CC = g++-8
CFLAGS = -Wall -Wextra -std=gnu++17
DEPENDENCIES.C = utils.c ext2.cpp imagereader.cpp bufferedimagereader.cpp
DIST = lab3a-705357270.tar.gz
MAIN.C = lab3a.cpp
MOUNT = fs
FILES = README Makefile lab3a.cpp utils.cpp utils.h ext2_fs.h ext2.cpp ext2.hpp
EXEC = lab3a
LIBS = -static-libstdc++


CFLAGS = -Wall -Wextra
DFLAGS = -g

default: lab3a

clean:
	rm -f $(EXEC) $(DIST)

debug: $(MAIN.C)
	$(CC) $(CFLAGS) -g $(MAIN.C) $(DEPENDENCIES.C) -o $(EXEC) $(LIBS)

dist: $(FILES)
	tar -czvf $(DIST) $(FILES)

lab3a: $(MAIN.C)
	$(CC) $(CFLAGS) $(MAIN.C) $(DEPENDENCIES.C) -o $(EXEC) $(LIBS)

mount:
	sudo mount -o ro,loop spec/EXT2_test.img $(MOUNT)

umount:
	sudo umount $(MOUNT)

test: dist
	cp lab3a-705357270.tar.gz test/ && cd test && ./P3A_check.sh 705357270

.PHONY: clean dist
CC = g++
CFLAGS = -Wall -Wextra -std=gnu++17
DFLAGS = -g
DEPENDENCIES.C = ext2.cpp imagereader.cpp bufferedimagereader.cpp
MAIN.C = main.cpp
MOUNT = fs
FILES = README bufferedimagereader.cpp bufferedimagereader.hpp ext2.cpp ext2.hpp ext2_fs.h imagereader.hpp imagereader.cpp lab3a.cpp Makefile metafile.hpp
EXEC = lab3a
LIBS = -static-libstdc++

default: main

clean:
	rm -f $(EXEC) $(DIST)

debug: $(MAIN.C)
	$(CC) $(CFLAGS) -g $(MAIN.C) $(DEPENDENCIES.C) -o $(EXEC) $(LIBS)

dist:
	tar -czvf $(DIST) $(FILES)

main: $(MAIN.C)
	$(CC) $(CFLAGS) $(MAIN.C) $(DEPENDENCIES.C) -o $(EXEC) $(LIBS)

# EXT2 Filesystem Image Validator

This project is based on an assignment for UCLA's undergraduate-level course -- Operating Systems Principles. 
The project objectives include:

- Reinforce the basic filesystem concepts of directory objects, file objects, and free space.
- Gain experience researching, examining, interpreting and processing information

# Details

The project is split into the following major components:

1) Main Program
2) EXT2 Class
3) ImageReader Class

The Main program doesn't do much. It parses the command line argument and
creates an EXT2 object. Once the object is instantiated, Main simply calls
methods tailored to the specification (one for each type of printout required in
the spec).


## EXT2 Class
- contains :ImageReader:
- contains :MetaFile:
- contains :vector<> groupDescTbl:

Upon construction, the EXT2 Class takes a filename and attempts to open it as if
it were an EXT2 image. We perform various checks on the incoming
string/filename, as well as on the image itself. The details of our checks and
error handling can be found in the "Error Handling" section below. After
checking the meta data from the image, the constructor proceeds to read the
Super Block, verifying that certain parameters are met, such as the "magic
number" assigned to EXT2 images (0xEF53). File reads are performed exclusively
by the BufferedImageReader class, which is described below.

After retrieving the meta file 'stat' and parsing and validating the Super
Block, we retrieve a full copy of the Group Descriptor Table and store it in a
std::vector<ext2_group_desc> object. This allows us to have rapid lookup of the
various groups (although the spec explicitly said there will only be one group
in the tests, we felt it would be wise to make it extensible). Successful
construction of the EXT2 object implies we have successfully read in the image
file, retrieved its metadata, parsed and validated the Super Block, and
collected a copy of the Group Descriptor Table in memory.

The rest of the EXT2 methods are dedicated to the extraction of specific data
fields contained within the file system.


## ImageReader (and BufferedImageReader) Class
The ImageReader parent class and BufferedImageReader sub-class are designed to
abstract disk reads from the higher level EXT2 class. It provides methods to
retrieve individual blocks or ranges of blocks. It also maintains a copy of the
superblock and group descriptor table in memory.


# Error Handling
We employed the try/catch mechanisms of C++ to deal with errors. The main
program (lab3a) contains two such try/catch blocks, the first of which deals
with initial memory allocation and file system initialization. Once the file
system is successfully read (i.e. after validating magic number, etc) the second
try/catch block deals with dynamic issues, such as corrupt data.


Summary of Block Size Impact
```
| Limits       | 1KiB          | 2KiB      | 4KiB       | 8KiB       |
|--------------+---------------+-----------+------------+------------|
| FSBlocks     | 2,147,483,647 | =         | =          | =          |
| BlksPerBG    | 8192          | 16348     | 32768      | 65536      |
| InodesPerBG  | ^             | ^         | ^          | ^          |
| BytesPerBG   | 8MiB          | 32MiB     | 128MiB     | 512MiB     |
| FSSizeReal   | 4TiB          | 8TiB      | 16TiB      | 32TiB      |
| FSSizeLnx    | 2TiB          | 8TiB      | 16TiB      | 32TiB      |
| BlksPerFile  | 16843020      | 134217728 | 1074791436 | 8594130956 |
| FileSizeReal | 16GiB         | 256GiB    | 2TiB       | 2TiB       |
| FileSizeLnx  | 16GiB         | 256GiB    | 2TiB       | 2TiB       |
```

# References
The following resources were instrumental in our research for this project.

EXT2 Internal Layout
by Dave Poirier

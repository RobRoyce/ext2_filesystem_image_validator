#!/usr/bin/env bash

# NAMES HERE TODO
# EMAILS HERE
# IDS HERE

log="log.out"

echo "Beginning Test Suite..."
echo "Full transcripts will be available in ./$log"

# Revision 0 Default
T=1
echo "--------------------------------------------------Beginning test $T [Revision 0, Default Options]"
dd if=/dev/zero of=./test.img bs=1K count=1K &>> $log
mkfs.ext2 -r 0 ./test.img &>> $log  <<EOF
y
EOF
./lab3a test.img &>> $log
ec1=$?

printf "Exit code: %d\n" $ec1
printf "Expected code: %d\n\n" 0
rm -f test.img


# Revision 1 Default
T=2
echo "--------------------------------------------------Beginning test $T [Revision 1, Default Options]"
dd if=/dev/zero of=./test.img bs=1K count=1K &>> $log
mkfs.ext2 -r 1 ./test.img &>> $log <<EOF
y
EOF

./lab3a test.img &>> $log
ec2=$?

printf "Exit code: %d\n" $ec2
printf "Expected code: %d\n\n" 0
rm -f test.img



# Multiple Block Groups
T=3
echo "--------------------------------------------------Beginning test $T [2+ Block Groups]"
dd if=/dev/zero of=./test.img bs=1K count=1K &>> $log
mkfs.ext2 -r 1 -g 512 ./test.img &>> $log <<EOF
y
EOF

./lab3a test.img &>> $log
ec3=$?

printf "Exit code: %d\n" $ec3
printf "Expected code: %d\n\n" 2
rm -f test.img



# Different inode sizes
T=4
echo "--------------------------------------------------Beginning test $T [inode.size != 128]"
dd if=/dev/zero of=./test.img bs=1K count=1K &>> $log
mkfs.ext2 -r 1 -I 256 ./test.img &>> $log <<EOF
y
EOF

./lab3a test.img &>> $log
ec4=$?

printf "Exit code: %d\n" $ec4
printf "Expected code: %d\n\n" 0
rm -f test.img




# Different inode sizes
T=5
echo "--------------------------------------------------Beginning test $T [block.size == 2K, Rev 0]"
dd if=/dev/zero of=./test.img bs=2K count=1K &>> $log
mkfs.ext2 -r 0 -b 2048 ./test.img &>> $log <<EOF
y
EOF

./lab3a test.img &>> $log
ec5=$?

printf "Exit code: %d\n" $ec5
printf "Expected code: %d\n\n" 0
rm -f test.img




# Different inode sizes
T=6
echo "--------------------------------------------------Beginning test $T [block.size == 3K, rev 1]"
dd if=/dev/zero of=./test.img bs=3K count=1K &>> $log
mkfs.ext2 -r 1 -b 3072 ./test.img &>> $log <<EOF
y
EOF

./lab3a test.img &>> $log
ec6=$?

printf "Exit code: %d\n" $ec6
printf "Expected code: %d\n\n" 0
rm -f test.img



# Different inode sizes
T=7
echo "--------------------------------------------------Beginning test $T [block.size == 4K, rev 0]"
dd if=/dev/zero of=./test.img bs=4K count=1K &>> $log
mkfs.ext2 -r 0 -b 4096 ./test.img &>> $log <<EOF
y
EOF

./lab3a test.img &>> $log
ec7=$?

printf "Exit code: %d\n" $ec7
printf "Expected code: %d\n\n" 0
rm -f test.img

# BLOCK_SIZE=1024
# BLOCKS_PER_GROUP=1024
# N_GROUPS=4
# INODE_SIZE=128
# N_INODES=420
# REV=1

# dd if=/dev/zero of=./test.img bs=1K count=4096
# mkfs.ext2 -b $BLOCK_SIZE -g $BLOCKS_PER_GROUP -I $INODE_SIZE -N $N_INODES -r $REV ./test.img


# Different inode sizes
T=8
echo "--------------------------------------------------Beginning test $T [block.size == 3K, rev 0]"
dd if=/dev/zero of=./test.img bs=3K count=1K &>> $log
mkfs.ext2 -r 0 -b 3072 ./test.img &>> $log <<EOF
y
EOF

./lab3a test.img &>> $log
ec8=$?

printf "Exit code: %d\n" $ec8
printf "Expected code: %d\n\n" 0
rm -f test.img

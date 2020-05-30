# #!/usr/bin/env bash

# # NAMES HERE
# # EMAILS HERE
# # IDS HERE

# # Revision 0 Default
# T=1
# echo "--------------------------------------------------Beginning test $T [Check Revision 0]"
# dd if=/dev/zero of=./test.img bs=1K count=1K 
# mkfs.ext2 -r 0 ./test.img  <<EOF
# y
# EOF

# res=`./lab3a test.img`
# ec1=$?

# printf "Exit code: %d\n" $ec1
# printf "Expected code: %d\n\n" 0
# rm -f test.img


# # Revision 1 Default
# T=2
# echo "--------------------------------------------------Beginning test $T [Check Revision 1]"
# dd if=/dev/zero of=./test.img bs=1K count=1K 
# mkfs.ext2 -r 1 ./test.img  <<EOF
# y
# EOF

# res=`./lab3a test.img`
# ec2=$?

# printf "Exit code: %d\n" $ec2
# printf "Expected code: %d\n\n" 0
# rm -f test.img



# # Multiple Block Groups
# T=3
# echo "--------------------------------------------------Beginning test $T [2+ Block Groups]"
# dd if=/dev/zero of=./test.img bs=1K count=1K 
# mkfs.ext2 -r 1 -g 512 ./test.img  <<EOF
# y
# EOF

# res=`./lab3a test.img`
# ec3=$?

# printf "Exit code: %d\n" $ec3
# printf "Expected code: %d\n\n" 2
# rm -f test.img



# Different inode sizes
T=4
echo "--------------------------------------------------Beginning test $T [inode.size != 128]"
dd if=/dev/zero of=./test.img bs=1K count=1K
mkfs.ext2 -r 1 -I 256 ./test.img  <<EOF
y
EOF

./lab3a test.img
ec4=$?

printf "Exit code: %d\n" $ec4
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

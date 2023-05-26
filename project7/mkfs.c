#include <stdlib.h>
#include <string.h>

#include "mkfs.h"
#include "free.h"
#include "block.h"
#include "inode.h"
#include "pack.h"

void mkfs(void)
{
    /*
    Write 1024 blocks of all zero bytes, sequentially, using the write() syscall to image_fd. This will make a 4096*1024-byte image.
    Call alloc() 7 times to allocate the first 7 blocks of the image. This will allocate the superblock, inode map, block map, 3 inode blocks, and 1 data block.
    */
    unsigned char *zero_block = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        zero_block[i] = 0;
    }
    for (int i = 0; i < 1024; i++)
    {
        bwrite(i, zero_block);
    }
    for (int i = 0; i < 7; i++)
    {
        alloc();
    }
    free(zero_block);

    // Create the root directory
    struct inode *root = ialloc();
    int dir_data_block_num = alloc();
    root->flags = DIR_FLAG_VALUE;
    root->size = 2 * ENTRY_SIZE; // 2 entries
    root->block_ptr[0] = dir_data_block_num;

    unsigned char block[BLOCK_SIZE];
    write_u16((char*)block, root->inode_num);
    unsigned char self_dir_name[16] = "."; 
    strcpy((char*)(block + 2), (char*)self_dir_name); 

    write_u16((char*)(block + ENTRY_SIZE), root->inode_num);
    unsigned char parent_dir_name[16] = ".."; 
    strcpy((char*)(block + ENTRY_SIZE + 2), (char*)parent_dir_name); 

    bwrite(dir_data_block_num, block);

    iput(root);
}
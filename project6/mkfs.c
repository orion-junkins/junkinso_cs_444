#include <stdlib.h>

#include "free.h"
#include "block.h"

void mkfs(void)
{
    /*
    Write 1024 blocks of all zero bytes, sequentially, using the write() syscall to image_fd. This will make a 4096*1024-byte image.
    Call alloc() 7 times to allocate the first 7 blocks of the image. This will allocate the superblock, inode map, block map, 3 inode blocks, and 1 data block.
    */
    //
    unsigned char *block = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        block[i] = 0;
    }
    for (int i = 0; i < 1024; i++)
    {
        bwrite(i, block);
    }
    for (int i = 0; i < 7; i++)
    {
        alloc();
    }
    free(block);
}
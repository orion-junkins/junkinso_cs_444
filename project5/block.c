#include <stdio.h>
#include <unistd.h>

#include "block.h"
#include "image.h"
#include "free.h"

unsigned char *bread(int block_num, unsigned char *block)
{
    /*
    Take a block number and a pointer to a block-sized unsigned char buffer to load the data into. For convenience, it should also return block at the end.
    */
    // Use the lseek() system call to move to the proper place within the image before you call read() or write() to load or save the block.
    lseek(image_fd, block_num * BLOCK_SIZE, SEEK_SET);
    read(image_fd, block, BLOCK_SIZE);

    return block;
}
void bwrite(int block_num, unsigned char *block)
{
    /*
    Takes a block number and a pointer to the data to write.
    */
    lseek(image_fd, block_num * BLOCK_SIZE, SEEK_SET);
    write(image_fd, block, BLOCK_SIZE);
}

int alloc(void)
{
    /*allocate a previous-free data block from the block map.*/
    unsigned char *data_map = bread(2, data_map);
    int free_data = find_free(data_map);
    if (free_data == -1){
        return -1;
    }
    set_free(data_map, free_data, 1);
    bwrite(1, data_map);
    return free_data;
}
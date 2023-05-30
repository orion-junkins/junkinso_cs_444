#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "inode.h"
#include "free.h"
#include "block.h"
#include "pack.h"

struct inode* ialloc(void)
{
    /*
    Allocate a previously-free inode in the inode map.
    */
    unsigned char *inode_map = malloc(BLOCK_SIZE);
    inode_map = bread(FREE_INODE_MAP_NUM, inode_map);
    int free_inode = find_free(inode_map);
    if (free_inode == -1)
    {
        return NULL;
    }
    set_free(inode_map, free_inode, 1);
    bwrite(FREE_INODE_MAP_NUM, inode_map);
    struct inode* new_node = iget(free_inode);
    new_node->size = 0;
    new_node->owner_id = 0;
    new_node->permissions = 0;
    new_node->flags = 0;
    for (int i=0; i < INODE_PTR_COUNT; i++){
        new_node->block_ptr[i] = 0;
    }
    write_inode(new_node);

    free(inode_map);

    return new_node;
}


struct inode *find_incore_free(void){
    /* 
    This finds the first free in-core inode in the incore array. It returns a pointer to that in-core inode or NULL if there are no more free in-core inodes.
    */
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++)
    {
        if (incore[i].ref_count == 0)
        {
            return &incore[i];
        }
    }

    return NULL;
}

struct inode *find_incore(unsigned int inode_num){
    /*
    This finds an in-core inode record in the incore array by the inode number. It returns a pointer to that in-core inode or NULL if it can't be found.
    */
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++)
    {
        if (incore[i].inode_num == inode_num && incore[i].ref_count > 0)
        {
            return &incore[i];
        }
    }
    return NULL;
}

void read_inode(struct inode *in, int inode_num){
    /*
    This takes a pointer to an empty struct inode that you're going to read the data into. The inode_num is the number of the inode you wish to read from disk.

    You'll have to map that inode number to a block and offset, as per above.

    Then you'll read the data from disk into a block, and unpack it with the functions from pack.c. And you'll store the results in the struct inode * that was passed in.
    */
    unsigned char *inode_block = malloc(BLOCK_SIZE);
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    inode_block = bread(block_num, inode_block);

    in->size = read_u32(inode_block + block_offset_bytes);
    in->owner_id = read_u16(inode_block + block_offset_bytes + 4);
    in->permissions = read_u8(inode_block + block_offset_bytes + 6);
    in->flags = read_u8(inode_block + block_offset_bytes + 7);
    in->link_count = read_u8(inode_block + block_offset_bytes + 8);
    for (int i = 0; i <= INODE_PTR_COUNT; i++)
    {
        int cur_block_offset = 9 + i * INODE_PTR_SIZE;
        in->block_ptr[i] = read_u16(inode_block + block_offset_bytes + cur_block_offset);
    }

    free(inode_block);
} 

void write_inode(struct inode *in) {
    /*
    This stores the inode data pointed to by in on disk. The inode_num field in the struct holds the number of the inode to be written.

    You'll have to map that inode number to a block and offset, as per above.

    Then you'll read the data from disk into a block, and pack the new inode data with the functions from pack.c. And lastly you'll write the updated block back out to disk.
    */
    unsigned char *inode_block = malloc(BLOCK_SIZE);

    int block_num = in->inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = in->inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;

    write_u32(inode_block + block_offset_bytes, in->size);
    write_u16(inode_block + block_offset_bytes + 4, in->owner_id);
    write_u8(inode_block + block_offset_bytes + 6, in->permissions);
    write_u8(inode_block + block_offset_bytes + 7, in->flags);
    write_u8(inode_block + block_offset_bytes + 8, in->link_count);
    for (int i = 0; i <= INODE_PTR_COUNT; i++)
    {
        int cur_block_offset = 9 + i * INODE_PTR_SIZE;
        write_u16(inode_block + block_offset_bytes + cur_block_offset, in->block_ptr[i]);
    }

    bwrite(block_num, inode_block);
    free(inode_block);

} 

struct inode *iget(int inode_num){
    /*
    Return a pointer to an in-core inode for the given inode number, or NULL on failure.
    */
    struct inode *in = find_incore(inode_num);
    if (in != NULL)
    {
        in->ref_count++;
        return in;
    }
    else
    {
        in = find_incore_free();
        if (in == NULL)
        {
            return NULL;
        }
        read_inode(in, inode_num);
        in->ref_count++;
        in->inode_num = inode_num;
        return in;
    }
}


void iput(struct inode *in) {
    /*
    Decrement the reference count on the inode. If it falls to 0, write the inode to disk.
    */
   if (in->ref_count == 0){
        return;
   }
   in->ref_count--;
   if (in->ref_count == 0){
       write_inode(in);
   }
}

struct inode *namei(char *path){
    /*
    Returns an in-core inode with a non-zero ref_count.
    NOTE: This current just returns the in-core inode for the root directory, /
    */
    return iget(ROOT_INODE_NUM);
}

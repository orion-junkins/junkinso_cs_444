#include <stdio.h>
#include <stdlib.h>

#include "inode.h"
#include "free.h"
#include "block.h"
static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

int ialloc(void)
{
    /*
    Allocate a previously-free inode in the inode map.
    */
    unsigned char *inode_map = malloc(BLOCK_SIZE);
    inode_map = bread(FREE_INODE_MAP_NUM, inode_map);
    int free_inode = find_free(inode_map);
    if (free_inode == -1)
    {
        return -1;
    }
    set_free(inode_map, free_inode, 1);
    bwrite(FREE_INODE_MAP_NUM, inode_map);
    free(inode_map);

    return free_inode;
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

#include "free.h"
#include "block.h"

int ialloc(void){
    /*
    Allocate a previously-free inode in the inode map.
    */
    unsigned char *inode_map = bread(1, inode_map);
    int free_inode = find_free(inode_map);
    if (free_inode == -1){
        return -1;
    }
    set_free(inode_map, free_inode, 1);
    bwrite(1, inode_map);
    
    return free_inode;
}

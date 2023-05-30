#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    Also allocate and populate the root directory node.
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

    // Write the first . entry to a new block
    unsigned char block[BLOCK_SIZE];
    write_u16((char *)block, root->inode_num);
    strcpy((char *)(block + INODE_NUMBER_SIZE), ".");

    // Add the second .. entry to the block
    write_u16((char *)(block + ENTRY_SIZE), root->inode_num);
    strcpy((char *)(block + ENTRY_SIZE + INODE_NUMBER_SIZE), "..");

    // Write the block to the root directory's data block
    bwrite(dir_data_block_num, block);

    // Write the root inode to disk
    iput(root);
}

struct directory *directory_open(int inode_num)
{
    /*
    This function takes an inode number and returns a struct directory *that can be used to read the entries in the directory.
    */
    // Get the inode
    struct inode *node = iget(inode_num);
    if (node == NULL)
    {
        return NULL;
    }
    // Malloc a new directory struct and fill it in
    struct directory *dir = malloc(sizeof(struct directory));
    dir->inode = node;
    dir->offset = 0;
    return dir;
}

int directory_get(struct directory *dir, struct directory_entry *ent)
{
    /*
    This function takes a struct directory *and a struct directory_entry *and fills in the directory entry with the next entry in the directory. It returns 0 on success and -1 on failure.
    */
    // Check the offset against the size of the directory
    if (dir->offset >= dir->inode->size)
    {
        return -1;
    }
    // Compute the block in the directory we need to read
    int data_block_index = dir->offset / 4096;

    // Read the appropriate data block in so we can extract the directory entry from it
    int data_block_num = dir->inode->block_ptr[data_block_index];
    unsigned char block[BLOCK_SIZE];
    bread(data_block_num, block);

    // Compute the offset of the directory entry in the block we just read.
    int offset_in_block = dir->offset % 4096;

    // Extract the inode number and name from the directory entry and store them in the struct directory_entry *ent.
    ent->inode_num = read_u16((char *)(block + offset_in_block));
    strcpy(ent->name, (char *)(block + offset_in_block + 2));

    // Increment the offset in the directory
    dir->offset = dir->offset + ENTRY_SIZE;
    return 0;
}

void directory_close(struct directory *d)
{
    /*
    This function takes a struct directory *, calls iput() on the inode associated with the directory and frees the struct directory *.
    */
    iput(d->inode);
    free(d);
}

char *get_dirname(const char *path, char *dirname)
{
    strcpy(dirname, path);

    char *p = strrchr(dirname, '/');

    if (p == NULL) {
        strcpy(dirname, ".");
        return dirname;
    }

    if (p == dirname)  // Last slash is the root /
        *(p+1) = '\0';

    else
        *p = '\0';  // Last slash is not the root /

    return dirname;
}

char *get_basename(const char *path, char *basename)
{
    if (strcmp(path, "/") == 0) {
        strcpy(basename, path);
        return basename;
    }

    const char *p = strrchr(path, '/');

    if (p == NULL)
        p = path;   // No slash in name, start at beginning
    else
        p++;        // Start just after slash

    strcpy(basename, p);

    return basename;
}


int directory_make(char *path) {
    /*
    Create a directory at the given path. Return 0 on success, -1 on failure.
    */
    // Find the directory path that will contain the new directory
    char dirname[strlen(path)];
    get_dirname(path, dirname);

    // Find the new directory name from the path
    char basename[strlen(path)];
    get_basename(path, basename);

    // Find the inode for the parent directory
    struct inode *parent = namei(dirname);

    // Create a new inode for the new directory
    struct inode *new_dir = ialloc();

    // Create a new data block for the new directory entries
    int new_dir_data_block = alloc();


    // Initialize the new directory in-core inode
    new_dir->flags = DIR_FLAG_VALUE;
    new_dir->size = 2 * ENTRY_SIZE; // 2 entries
    new_dir->block_ptr[0] = new_dir_data_block;

    // Create a new block-sized array for the new directory data block and initialize it with the . and .. entries
    unsigned char new_dir_block[BLOCK_SIZE];
    write_u16((char *)new_dir_block, new_dir->inode_num);
    strcpy((char *)(new_dir_block + INODE_NUMBER_SIZE), ".");

    write_u16((char *)(new_dir_block + ENTRY_SIZE), new_dir->inode_num);
    strcpy((char *)(new_dir_block + ENTRY_SIZE + INODE_NUMBER_SIZE), "..");

    // Write the new directory data new_dir_block to disk (bwrite()).
    bwrite(new_dir_data_block, new_dir_block);

    // Find the block that will contain the new directory entry
    int parent_block_num = parent->size / BLOCK_SIZE;

    // Read that block into memory
    unsigned char root_block[BLOCK_SIZE];
    bread(parent->block_ptr[parent_block_num], root_block);

    // Add the new directory entry to it.
    int offset_in_block = parent->size % BLOCK_SIZE;
    write_u16((char *)(root_block + offset_in_block), new_dir->inode_num);
    strcpy((char *)(root_block + offset_in_block + INODE_NUMBER_SIZE), basename);

    // Write that block to disk
    bwrite(parent->block_ptr[parent_block_num], root_block);

    // Update the parent directory's size field 
    parent->size = parent->size + ENTRY_SIZE;

    // Release the new directory's in-core inode
    iput(new_dir);

    // Release the parent directory's in-core inode
    iput(parent);

    return 0;
}

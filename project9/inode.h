#ifndef INODE_H
#define INODE_H

#define FREE_INODE_MAP_NUM 1
#define INODE_PTR_COUNT 16
#define INODE_PTR_SIZE 2
#define MAX_SYS_OPEN_FILES 64
#define BLOCK_SIZE 4096
#define INODE_SIZE 64
#define INODE_FIRST_BLOCK 3
#define ROOT_INODE_NUM 0
#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)

struct inode {
    unsigned int size;
    unsigned short owner_id;
    unsigned char permissions;
    unsigned char flags;
    unsigned char link_count;
    unsigned short block_ptr[INODE_PTR_COUNT];

    unsigned int ref_count;  // in-core only
    unsigned int inode_num;
};

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

struct inode* ialloc(void);

struct inode * find_incore_free(void);

struct inode *find_incore(unsigned int inode_num);

void read_inode(struct inode *in, int inode_num);
void write_inode(struct inode *in);
struct inode *iget(int inode_num);
void iput(struct inode *in);
struct inode *namei(char *path);

#endif
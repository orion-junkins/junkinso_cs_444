#ifndef MKFS_H
#define MKFS_H

#define DIR_FLAG_VALUE 2
#define ENTRY_SIZE 32
#define INODE_NUMBER_SIZE 2

struct directory {
    struct inode *inode;
    unsigned int offset;
};

struct directory_entry {
    unsigned int inode_num;
    char name[16];
};

void mkfs(void);
void directory_close(struct directory *d);
int directory_get(struct directory *dir, struct directory_entry *ent);
struct directory *directory_open(int inode_num);
char *get_dirname(const char *path, char *dirname);
char *get_basename(const char *path, char *basename);
int directory_make(char *path);
#endif
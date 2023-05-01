unsigned char *bread(int block_num, unsigned char *block);
/*
Take a block number and a pointer to a block-sized unsigned char buffer to load the data into. For convenience, it should also return block at the end.
*/
void bwrite(int block_num, unsigned char *block);
/*
bwrite() takes a block number and a pointer to the data to write.
*/

int alloc(void);
/*allocate a previous-free data block from the block map.*/
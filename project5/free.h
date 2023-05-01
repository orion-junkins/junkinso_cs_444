#ifndef FREE_H
#define FREE_H
void set_free(unsigned char *block, int num, int set);
/* set a specific bit to the value in set (0 or 1)*/
void find_free(unsigned char *block);
/*find a 0 bit and return its index (i.e. the block number that corresponds to this bit.*/

#endif
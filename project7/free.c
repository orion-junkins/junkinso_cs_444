#include "free.h"
#include "block.h"

void set_free(unsigned char *block, int num, int set)
{
    /*
    Set the bit at the given number in the block to 1 or 0
    */
    int byte_num = num / 8; // 8 bits per byte
    int bit_num = num % 8;
    unsigned char mask = 1 << bit_num;
    if (set == 1)
    {
        block[byte_num] = block[byte_num] | mask;
    }
    else
    {
        block[byte_num] = block[byte_num] & ~mask;
    }
}

int find_low_clear_bit(unsigned char x)
{
    /*
    Find the lowest clear bit in a byte
    Return -1 if all bits are set
    Return the bit number if there is a clear bit
    */
    for (int i = 0; i < 8; i++)
        if (!(x & (1 << i)))
            return i;

    return -1;
}

int find_free(unsigned char *block)
{
    /*
    Find the lowest clear bit in a block
    Return -1 if all bits are set
    Return the bit number if there is a clear bit
    */
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        int low_clear_bit = find_low_clear_bit(block[i]);
        if (low_clear_bit != -1)
        {
            return i * 8 + low_clear_bit;
        }
    }
    return -1;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "inode.h"
#include "mkfs.h"

void test_file_creation(void)
{
    image_open("image", 1);
    CTEST_ASSERT(image_fd != -1, "testing file is open");
    image_close();
    CTEST_ASSERT(access("image", F_OK) == 0, "testing file creation");
}

void test_bread_and_bwrite(void)
{
    image_open("image", 1);

    unsigned char *block_a_to_write = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        block_a_to_write[i] = 'a';
    }
    bwrite(7, block_a_to_write);

    unsigned char *block_b_to_write = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        block_b_to_write[i] = 'b';
    }
    bwrite(8, block_b_to_write);

    unsigned char *block_a_to_read = malloc(BLOCK_SIZE);
    bread(7, block_a_to_read);

    unsigned char *block_b_to_read = malloc(BLOCK_SIZE);
    bread(8, block_b_to_read);

    CTEST_ASSERT(memcmp(block_a_to_read, block_a_to_write, BLOCK_SIZE) == 0, "testing bread and bwrite");
    CTEST_ASSERT(memcmp(block_b_to_read, block_b_to_write, BLOCK_SIZE) == 0, "testing bread and bwrite");
    CTEST_ASSERT(memcmp(block_a_to_read, block_b_to_write, BLOCK_SIZE) != 0, "testing bread and bwrite");

    free(block_a_to_write);
    free(block_b_to_write);
    free(block_a_to_read);
    free(block_b_to_read);
    image_close();
}

void test_set_and_find_free(void)
{
    unsigned char *testing_block = malloc(BLOCK_SIZE);
    set_free(testing_block, 0, 1);
    set_free(testing_block, 1, 1);
    set_free(testing_block, 2, 1);
    set_free(testing_block, 4, 1);
    set_free(testing_block, 5, 1);

    CTEST_ASSERT(find_free(testing_block) == 3, "testing set_free and find_free");
    set_free(testing_block, 3, 1);
    set_free(testing_block, 4, 0);

    CTEST_ASSERT(find_free(testing_block) == 4, "testing set_free and find_free");
    free(testing_block);
}

void test_ialloc(void)
{
    image_open("image", 1);

    unsigned char *testing_block = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        set_free(testing_block, i, 0);
    }
    bwrite(1, testing_block);

    int inode_0_num = ialloc();
    CTEST_ASSERT(inode_0_num == 0, "testing ialloc");
    int inode_1_num = ialloc();
    CTEST_ASSERT(inode_1_num == 1, "testing ialloc");

    free(testing_block);
    image_close();
}

void test_alloc(void)
{
    image_open("image", 1);

    unsigned char *testing_block = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        set_free(testing_block, i, 0);
    }
    bwrite(1, testing_block);

    int inode_0_num = alloc();
    CTEST_ASSERT(inode_0_num == 0, "testing ialloc");
    int inode_1_num = alloc();
    CTEST_ASSERT(inode_1_num == 1, "testing ialloc");

    free(testing_block);
    image_close();
}

void test_mkfs(void)
{
    image_open("image", 1);
    mkfs();

    unsigned char *testing_block = malloc(BLOCK_SIZE);
    for (int i = 0; i < 7; i++)
    {
        testing_block = bread(i, testing_block);
        if (i == FREE_BLOCK_MAP_NUM)
        {
            // 0-6 7 should be set to 1 in the data block
            CTEST_ASSERT(find_free(testing_block) == 7, "testing set_free and find_free");
        }
        else
        {
            CTEST_ASSERT(find_free(testing_block) == 0, "testing set_free and find_free");
        }
    }

    unsigned char *expected_empty_block = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        expected_empty_block[i] = 0;
    }

    unsigned char *current_block = malloc(BLOCK_SIZE);
    int all_match = 1;
    for (int i = 3; i < 5024; i++)
    {
        current_block = bread(i, current_block);
        if (memcmp(expected_empty_block, current_block, BLOCK_SIZE) != 0)
        {
            all_match = 0;
        }
    }

    CTEST_ASSERT(all_match == 1, "testing all blocks are inode and file data empty");

    free(testing_block);
    free(expected_empty_block);
    free(current_block);
    image_close();
}

int main(void)
{
    CTEST_VERBOSE(1);

    test_file_creation();
    test_bread_and_bwrite();
    test_set_and_find_free();
    test_ialloc();
    test_alloc();
    test_mkfs();

    CTEST_RESULTS();

    CTEST_EXIT();
}

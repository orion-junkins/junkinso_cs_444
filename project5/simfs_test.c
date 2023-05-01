#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"

void test_file_creation(void)
{
    image_open("image", 0);
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
    image_close();
}

void test_set_and_find_free(void){
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
}
int main(void)
{
    CTEST_VERBOSE(1);

    test_file_creation();
    test_bread_and_bwrite();
    test_set_and_find_free();

    CTEST_RESULTS();

    CTEST_EXIT();
}
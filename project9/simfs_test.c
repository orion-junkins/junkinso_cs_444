#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "inode.h"
#include "ls.h"
#include "mkfs.h"

void clear_incore_inodes(void)
{
    memset(incore, 0, sizeof incore);
}

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
    image_open("image", 1);

    unsigned char *testing_block = malloc(BLOCK_SIZE);
    set_free(testing_block, 0, 1);
    set_free(testing_block, 1, 1);
    set_free(testing_block, 2, 1);
    set_free(testing_block, 4, 1);
    set_free(testing_block, 5, 1);

    CTEST_ASSERT(find_free(testing_block) == 3, "testing find_free finds first free block");
    set_free(testing_block, 3, 1);
    set_free(testing_block, 4, 0);

    CTEST_ASSERT(find_free(testing_block) == 4, "testing find_free finds a block that has been previously allocated then freed");
    free(testing_block);

    image_close();
}

void test_ialloc(void)
{
    image_open("image", 1);

    struct inode *node_0 = ialloc();
    CTEST_ASSERT(node_0->size == 0, "test ialloc defaults to size of 0");
    CTEST_ASSERT(node_0->owner_id == 0, "test ialloc defaults to owner_id of 0");
    CTEST_ASSERT(node_0->permissions == 0, "test ialloc defaults to permissions of 0");
    CTEST_ASSERT(node_0->flags == 0, "test ialloc defaults to flags of 0");

    struct inode *node_1 = ialloc();

    CTEST_ASSERT(node_1->inode_num != node_0->inode_num, "test subsequent calls to ialloc yield different inode numbers");

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
    bwrite(FREE_BLOCK_MAP_NUM, testing_block);

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
            // 7 should be free in the free block map
            CTEST_ASSERT(find_free(testing_block) == 8, "testing 8 is the first free index in the Free Block Map");
        }
        else if (i == FREE_INODE_MAP_NUM)
        {
            CTEST_ASSERT(find_free(testing_block) == 1, "testing 1 is the first free index in the Free Inode Map (root allocated)");
        }
        else
        {
            CTEST_ASSERT(find_free(testing_block) == 0, "testing all other blocks are empty"); // fail
        }
    }

    unsigned char *expected_empty_block = malloc(BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        expected_empty_block[i] = 0;
    }

    unsigned char *current_block = malloc(BLOCK_SIZE);
    int all_match = 1;
    for (int i = 4; i < 7; i++)
    {
        current_block = bread(i, current_block);
        if (memcmp(expected_empty_block, current_block, BLOCK_SIZE) != 0)
        {
            all_match = 0;
        }
    }

    for (int i = 8; i < 5024; i++)
    {
        current_block = bread(i, current_block);
        if (memcmp(expected_empty_block, current_block, BLOCK_SIZE) != 0)
        {
            all_match = 0;
        }
    }

    CTEST_ASSERT(all_match == 1, "testing all blocks except inode data block 0 (block 3) and file data block 0 (block 7) are empty");

    free(testing_block);
    free(expected_empty_block);
    free(current_block);

    image_close();
}

void test_incore_inodes(void)
{
    image_open("image", 1);
    // Find a free inode in the incore inode table
    struct inode *original_node = find_incore_free();

    // Modify it to set some values
    original_node->inode_num = 27;
    original_node->ref_count = 1;
    original_node->owner_id = 1;

    // Search for that by number
    struct inode *new_node = find_incore(27);

    // Verify that the new node has the same data as the original node.
    CTEST_ASSERT(new_node->inode_num == original_node->inode_num, "testing incore inode has expected inode number");
    CTEST_ASSERT(new_node->owner_id == original_node->owner_id, "testing incore inodes have expected owner id");
    image_close();
}

void test_inode_read_and_write(void)
{
    image_open("image", 1);

    struct inode *node = malloc(sizeof(struct inode));
    node->size = 1;
    node->owner_id = 1;
    node->permissions = 1;
    node->flags = 1;
    node->link_count = 1;
    node->block_ptr[0] = 1;
    node->block_ptr[1] = 1;
    node->inode_num = 1;
    write_inode(node);
    free(node);

    struct inode *new_node = malloc(sizeof(struct inode));
    read_inode(new_node, 1);

    CTEST_ASSERT(new_node->size == node->size, "testing inode reads right size from written inode");
    CTEST_ASSERT(new_node->owner_id == node->owner_id, "testing inode reads right owner_id from written inode");
    CTEST_ASSERT(new_node->permissions == node->permissions, "testing inode reads right permissions from written inode");
    CTEST_ASSERT(new_node->flags == node->flags, "testing inode reads right flags from written inode");
    CTEST_ASSERT(new_node->link_count == node->link_count, "testing inode reads right link_count from written inode");
    CTEST_ASSERT(new_node->block_ptr[0] == node->block_ptr[0], "testing inode reads right block_ptr[0] from written inode");
    CTEST_ASSERT(new_node->block_ptr[1] == node->block_ptr[1], "testing inode reads right block_ptr[1] from written inode");
    free(new_node);

    image_close();
}

void test_inode_get_and_put(void)
{
    image_open("image", 1);

    struct inode *node = malloc(sizeof(struct inode));
    node->size = 1;
    node->owner_id = 1;
    node->permissions = 1;
    node->flags = 1;
    node->link_count = 1;
    node->block_ptr[0] = 1;
    node->block_ptr[1] = 1;
    node->inode_num = 1;
    node->ref_count = 1;
    iput(node);

    struct inode *new_node = iget(1);
    CTEST_ASSERT(new_node->size == node->size, "testing inode get recieves correct size from inode put");
    CTEST_ASSERT(new_node->owner_id == node->owner_id, "testing inode get recieves correct owner_id from inode put");
    CTEST_ASSERT(new_node->permissions == node->permissions, "testing inode get recieves correct permissions from inode put");
    CTEST_ASSERT(new_node->flags == node->flags, "testing inode get recieves correct flags from inode put");
    CTEST_ASSERT(new_node->link_count == node->link_count, "testing inode get recieves correct link_count from inode put");
    CTEST_ASSERT(new_node->block_ptr[0] == node->block_ptr[0], "testing inode get recieves correct block_ptr[0] from inode put");
    CTEST_ASSERT(new_node->block_ptr[1] == node->block_ptr[1], "testing inode get recieves correct block_ptr[1] from inode put");
    free(node);

    image_close();
}

void test_directory_open_close(void)
{
    image_open("image", 1);
    mkfs();
    struct directory *root = directory_open(0);
    CTEST_ASSERT(root->inode->inode_num == 0, "testing directory_open returns expected inode number");
    CTEST_ASSERT(root->inode->flags == DIR_FLAG_VALUE, "testing directory_open returns a directory");
    CTEST_ASSERT(root->inode->size == 2 * ENTRY_SIZE, "testing directory_open returns a directory with the expected size");
    CTEST_ASSERT(root->inode->block_ptr[0] == 7, "testing directory_open returns a directory with the expected data block");
    directory_close(root);
    CTEST_ASSERT(root->inode == NULL, "testing directory_close frees the inode");
    image_close();
}

void test_directory_open_failure(void)
{
    // Make sure incore is clear
    clear_incore_inodes();

    // Fill incore with inodes
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++)
    {
        iget(i);
    }

    struct directory *root = directory_open(MAX_SYS_OPEN_FILES);
    CTEST_ASSERT(root == NULL, "testing directory_open returns NULL when there are no free inodes");
}

void test_directory_get(void)
{
    // Mirrors the behavior of ls() to verify that all directory functions are working together as expected
    image_open("image", 1);
    mkfs();

    struct directory *dir;
    struct directory_entry ent;

    dir = directory_open(0);
    int first_open = directory_get(dir, &ent);
    CTEST_ASSERT(first_open == 0, "testing directory_get returns 0 when first entry successfully opened");
    CTEST_ASSERT(ent.inode_num == 0, "testing directory_get returns the expected inode number");
    CTEST_ASSERT(strcmp(ent.name, ".") == 0, "testing directory_get returns the expected name");

    int second_open = directory_get(dir, &ent);
    CTEST_ASSERT(second_open == 0, "testing directory_get returns 0 when second entry successfully opened");
    CTEST_ASSERT(ent.inode_num == 0, "testing directory_get returns the expected inode number");
    CTEST_ASSERT(strcmp(ent.name, "..") == 0, "testing directory_get returns the expected name");

    int third_open = directory_get(dir, &ent);
    CTEST_ASSERT(third_open == -1, "testing directory_get returns -1 when there are no more entries");

    image_close();
}

void test_directory_make(void)
{
    // Mirrors the behavior of ls() to verify that all directory functions are working together as expected
    image_open("image", 1);
    mkfs();

    struct directory *dir;
    struct directory_entry ent;

    int make_dir_0 = directory_make("/new_dir_0");
    CTEST_ASSERT(make_dir_0 == 0, "testing directory_make returns 0 when directory is successfully created");

    int make_dir_1 = directory_make("/new_dir_1");
    CTEST_ASSERT(make_dir_1 == 0, "testing directory_make returns 0 when directory is successfully created");

    dir = directory_open(0);
    int first_open = directory_get(dir, &ent);
    CTEST_ASSERT(first_open == 0, "testing directory_get returns 0 when first entry successfully opened");
    CTEST_ASSERT(ent.inode_num == 0, "testing directory_get returns the expected inode number");
    CTEST_ASSERT(strcmp(ent.name, ".") == 0, "testing directory_get returns the expected name");

    int second_open = directory_get(dir, &ent);
    CTEST_ASSERT(second_open == 0, "testing directory_get returns 0 when second entry successfully opened");
    CTEST_ASSERT(ent.inode_num == 0, "testing directory_get returns the expected inode number");
    CTEST_ASSERT(strcmp(ent.name, "..") == 0, "testing directory_get returns the expected name");

    int third_open = directory_get(dir, &ent);
    CTEST_ASSERT(third_open == 0, "testing directory_get returns 0 when third entry successfully opened");
    CTEST_ASSERT(strcmp(ent.name, "new_dir_0") == 0, "testing directory_get returns the expected name");

    int fourth_open = directory_get(dir, &ent);
    CTEST_ASSERT(fourth_open == 0, "testing directory_get returns 0 when fourth entry successfully opened");
    CTEST_ASSERT(strcmp(ent.name, "new_dir_1") == 0, "testing directory_get returns the expected name");

    int final_open = directory_get(dir, &ent);
    CTEST_ASSERT(final_open == -1, "testing directory_get returns -1 when there are no more entries");

    image_close();
}

void test_directory_make_fails_inodes_full(void)
{
    image_open("image", 1);
    mkfs();

    unsigned char inode_map[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE * 8; i++)
    {
        set_free(inode_map, i, 1);
    }
    bwrite(FREE_INODE_MAP_NUM, inode_map);

    int make_dir_0 = directory_make("/new_dir_0");

    CTEST_ASSERT(make_dir_0 == -1, "testing directory_make returns -1 when there are no free inodes");

    image_close();
}

void test_directory_make_fails_data_blocks_full(void)
{
    image_open("image", 1);
    mkfs();

    unsigned char data_block_map[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE * 8; i++)
    {
        set_free(data_block_map, i, 1);
    }
    bwrite(FREE_BLOCK_MAP_NUM, data_block_map);

    int make_dir_0 = directory_make("/new_dir_0");

    CTEST_ASSERT(make_dir_0 == -1, "testing directory_make returns -1 when there are no free data blocks");

    image_close();
}

void test_find_free_failure(void)
{
    unsigned char testing_block[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE * 8; i++)
    {
        set_free(testing_block, i, 1);
    }

    int free_inode = find_free(testing_block);
    CTEST_ASSERT(free_inode == -1, "testing find_free returns -1 when there is nothing free in the block");
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
    test_incore_inodes();
    test_inode_read_and_write();
    test_inode_get_and_put();
    test_directory_open_close();
    test_directory_open_failure();
    test_directory_get();
    test_directory_make();
    test_directory_make_fails_inodes_full();
    test_directory_make_fails_data_blocks_full();
    test_find_free_failure();
    CTEST_RESULTS();

    CTEST_EXIT();
}

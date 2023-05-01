#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ctest.h"
#include "image.h"

void test_file_creation(void)
{
    image_open("image", 0);
    image_close();
    CTEST_ASSERT(access("image", F_OK) == 0, "testing file creation");
}

int main(void)
{
    CTEST_VERBOSE(1);

    test_file_creation();

    CTEST_RESULTS();

    CTEST_EXIT();
}
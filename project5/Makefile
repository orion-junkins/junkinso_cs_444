.PHONY: all clean

simfs.a: block.o free.o image.o inode.o mkfs.o
	ar rcs $@ $^

simfs_test: simfs_test.c simfs.a
	gcc -Wall -Wextra -DCTEST_ENABLE -o $@ $^

clean:
	rm -f *.o *.a simfs_test

.PHONY: test

test: simfs_test
	./simfs_test


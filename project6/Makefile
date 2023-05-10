simfs_test: simfs_test.o simfs.a
	gcc -o $@ $^  # build executable

simfs_test.o: simfs_test.c
	gcc -Wall -Wextra -c -DCTEST_ENABLE $<

simfs.a: block.o free.o image.o inode.o mkfs.o
	ar rcs $@ $^

block.o: block.c
	gcc -Wall -Wextra -c $<

free.o: free.c
	gcc -Wall -Wextra -c $<

image.o: image.c
	gcc -Wall -Wextra -c $<

inode.o: inode.c
	gcc -Wall -Wextra -c $<

mkfs.o: mkfs.c
	gcc -Wall -Wextra -c $<

.PHONY: clean test

clean:
	rm -f *.o

test: simfs_test
	./simfs_test


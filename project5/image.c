#include <fcntl.h>
#include <unistd.h>
#include "image.h"

// A global variable to represent the open image file. This variable will be set within image_open:
int image_fd;

int image_open(char *filename, int truncate){
// 
/* 
Open the image file of the given name, creating it if it doesn't exist, and truncating it to 0 size if truncate is true (see below).
*/
    int flags = O_RDWR | O_CREAT;
    if(truncate){
        flags |= O_TRUNC;
    }
    image_fd = open(filename, flags, 0600);
    if(image_fd == -1){
        return -1;
    }
    return 0;
}

int image_close(){
    // This closes the image file.
    return close(image_fd);
}

#include <stdio.h>
#include <stdlib.h>

// The includes listed in the lstat man page.
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int main(int argc, char **argv) {

    // In order for this demo to work, create a file named example.txt in the 
    // same directory as this code.
    char *path = "example.txt";

    // On teach.cs, run: man lstat
    // Read about the lstat system call, which has two parameters:
    // a file path and a pointer to a struct stat.

    // Note: lstat is available on teach.cs and it may not be installed on
    // your personal computer.  Run this demo on teach.cs.

    // We need to allocate space for a struct stat.  We'll pass
    // a pointer to this struct as an argument to lstat, and
    // the call on lstat will populate it with information about path.
    struct stat stat_buf;
    
    // Call lstat to populate stat_buf with information about path. 
    // Also, error check the system call and if it fails, return with exit code 1.
    // See the RETURN section of the man page to learn about what lstat returns.
    if (lstat(path, &stat_buf) == -1) {
        perror("lstat");
        return 1;
    }

    // Let's use the information in stat_buf to check whether path is a
    // regular file. Read the lstat man page to find out about the various
    // macros that you can use.

    // We'll use macro S_ISREG:
    if (S_ISREG(stat_buf.st_mode)) {
    	printf("%s is a regular file\n", path);
    }

    // Next, we'll use use the logical "and" operator to extract the permissions:
    int permissions = stat_buf.st_mode & 0777;
    printf("the file permissions are %o\n", permissions);
    
    return 0;

}



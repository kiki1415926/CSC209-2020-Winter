#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main() {

    // We're going to use opendir and readdir to access files in a directory.

    // In order for this demo to work, create a directory named mydir in the
    // same directory as this code.  In mydir, create two regular files named
    // a.txt and b.txt.

    // Call opendir, which returns a DIR pointer.
    char *path = "mydir";
    DIR *d_ptr = opendir(path);

    // Error check the system call (read about the return value in the man page)
    if (d_ptr == NULL) {
        perror("opendir");
        return 1;
    }

    // Allocate a struct dirent pointer, which will be used to store information
    // about a directory entry.
    struct dirent *entry_ptr;

    // Call readdir to get struct dirent, which represents a directory entry.  The
    // order that entries are returned by readdir is not guaranteed.  
    entry_ptr = readdir(d_ptr);

    printf("Directory %s contains:\n", path);
    while (entry_ptr != NULL) {
        printf("%s\n", entry_ptr->d_name);
        entry_ptr = readdir(d_ptr);
    }

    // TODO: read the man page to see what closedir returns on error, and complete
    // the error checking for this function call.
    closedir(d_ptr);

    return 0;           
}
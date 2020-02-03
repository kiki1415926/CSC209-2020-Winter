#include <stdio.h>
// Add your system includes here.

#include "ftree.h"
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

/*
 * Returns the FTree rooted at the path fname.
 *
 * Use the following if the file fname doesn't exist and return NULL:
 * fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
 *
 */

struct TreeNode *generate_ftree(const char *fname) {

    // Your implementation here.

    // Hint: consider implementing a recursive helper function that
    // takes fname and a path.  For the initial call on the 
    // helper function, the path would be "", since fname is the root
    // of the FTree.  For files at other depths, the path would be the
    // file path from the root to that file.
    struct stat stat_buf;
    if (lstat(fname, &stat_buf) == -1) {
        fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
        return NULL;
    }
    struct TreeNode *tn_ptr = malloc(sizeof(struct TreeNode));
    strcpy(tn_ptr -> fname, fname);
    tn_ptr -> permissions = stat_buf.st_mode & 0777;

    if (S_ISREG(stat_buf.st_mode)) {
    	tn_ptr -> type = '-';
        tn_ptr -> contents = NULL;
    } else if (S_ISDIR(stat_buf.st_mode)){
        tn_ptr -> type = 'd';
        DIR *d_ptr = opendir(fname);
        if (d_ptr == NULL) {
            perror("opendir");
            return NULL;
        }
        struct dirent *entry_ptr;
        entry_ptr = readdir(d_ptr);
        struct TreeNode *prev = NULL;
        while (entry_ptr != NULL) {
            struct TreeNode *curr_tn = generate_ftree(entry_ptr->d_name);
            tn_ptr -> contents = curr_tn;
            prev -> next = curr_tn;
            prev = curr_tn;
            entry_ptr = readdir(d_ptr);
        }
    } else if (S_ISLNK(stat_buf.st_mode)){
        tn_ptr -> type = 'l';
        tn_ptr -> contents = NULL;
    } else {
        fprintf(stderr, "The path (%s) is undefined type.\n", fname);
        free(tn_ptr);
        return NULL;
    }
    return tn_ptr;
}


/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 *
 * The only print statements that you may use in this function are:
 * printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions)
 * printf("%s (%c%o)\n", root->fname, root->type, root->permissions)
 *
 */
void print_ftree(struct TreeNode *root) {
	
    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.
    static int depth = 0;
    printf("%*s", depth * 2, "");
    // Your implementation here.
    if (root -> type == 'l' || root -> type == '-'){
        printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
    }else{
        printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
        struct TreeNode *curr = root -> contents;
        while (curr != NULL){
            printf("  ");
            print_ftree(curr);
            curr = curr -> next;
        }
    }
}


/* 
 * Deallocate all dynamically-allocated memory in the FTree rooted at node.
 * 
 */
void deallocate_ftree (struct TreeNode *node) {
   
   // Your implementation here.

}

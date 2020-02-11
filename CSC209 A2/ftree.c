#include <stdio.h>
// Add your system includes here.

#include "ftree.h"
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

struct TreeNode *generate_ftree_with_path(const char *fname, const char *path) {
    struct stat stat_buf;
    char path_buf[256];
    printf("path: %s\n", path);
    printf("fname: %s\n", fname);
    sprintf(path_buf, "%s%s", path, fname);
    if (lstat(path_buf, &stat_buf) == -1) {
        fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
        return NULL;
    }
    struct TreeNode *tn_ptr = malloc(sizeof(struct TreeNode));
    tn_ptr -> fname = malloc(strlen(fname) + 1);
    strcpy(tn_ptr -> fname, fname);

    tn_ptr -> permissions = stat_buf.st_mode & 0777;

    if (S_ISREG(stat_buf.st_mode)) {
    	tn_ptr -> type = '-';
        tn_ptr -> contents = NULL;
    } else if (S_ISDIR(stat_buf.st_mode)){
        tn_ptr -> type = 'd';
        strcat(path_buf, "/");
        DIR *d_ptr = opendir(path_buf);
        if (d_ptr == NULL) {
            perror("opendir");
            return NULL;
        }
        struct dirent *entry_ptr;
        entry_ptr = readdir(d_ptr);
        struct TreeNode *prev = NULL;
        while (entry_ptr != NULL) {
            if (entry_ptr->d_name[0] == '.') {
                entry_ptr = readdir(d_ptr);
                continue;
            }
            printf("%s\n", path_buf);
            
            struct TreeNode *curr_tn = generate_ftree_with_path(entry_ptr->d_name, path_buf);
            if (prev == NULL) {
                tn_ptr -> contents = curr_tn;
            } else {
                prev -> next = curr_tn;
            }
            prev = curr_tn;
            entry_ptr = readdir(d_ptr);
        }
        closedir(d_ptr);
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

    return generate_ftree_with_path(fname, "");
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
        depth++;
        struct TreeNode *curr = root -> contents;
        while (curr != NULL){
            // printf("  ");
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
    if (node -> type != 'd'){
        free(node -> fname);
        free(node);
    }else{
        struct TreeNode *curr = node -> contents;
        struct TreeNode *tmp = NULL;
        while (curr != NULL){
            tmp = curr;
            curr = curr -> next;
            deallocate_ftree(tmp);
        }
        free(node -> fname);
        free(node);
    }

}

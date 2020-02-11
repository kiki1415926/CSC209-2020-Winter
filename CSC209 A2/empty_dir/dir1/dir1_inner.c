#include <stdio.h>
// Add your system includes here.
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "ftree.h"


/*
 * Returns the FTree rooted at the path fname.
 *
 * Use the following if the file fname doesn't exist and return NULL:
 * fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
 *
 */
struct TreeNode *generate_ftree_helper(const char *name, const char *path);
struct TreeNode *generate_ftree(const char *fname) {

    // Your implementation here.

    // Hint: consider implementing a recursive helper function that
    // takes fname and a path.  For the initial call on the 
    // helper function, the path would be "", since fname is the root
    // of the FTree.  For files at other depths, the path would be the
    // file path from the root to that file.
    return generate_ftree_helper(fname, fname);
}

struct TreeNode *generate_ftree_helper(const char *name, const char *path) {
    printf("name %s, path %s\n", name, path);
    struct stat stat_buf;
    if (lstat(path, &stat_buf) == -1) {
        perror("lstat");
        exit (1);
    }
    struct TreeNode *tnode = malloc(sizeof(struct TreeNode));
    if(tnode == NULL) {
        perror("malloc");
        exit (1);
    }
    // assign fname
    tnode->fname = malloc(sizeof(char)*(strlen(name)+1));
    if(tnode->fname == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(tnode->fname, name);
    // assign permissions
    tnode->permissions = stat_buf.st_mode & 0777;
    tnode->contents = NULL;
    tnode->next = NULL;
    // assign type and contents according to file's type
    if (S_ISREG(stat_buf.st_mode)) { //regular file
        tnode->type = '-';
        // printf("file %s\n", name);
    } else if (S_ISLNK(stat_buf.st_mode)) { //linked file
        tnode->type = 'l';
        // printf("linked file\n");
    } else if (S_ISDIR(stat_buf.st_mode)) { //directory
        tnode->type = 'd';
        // printf("directory %s\n", name);
        DIR *d_ptr = opendir(path);
        if (d_ptr == NULL) {
            perror("opendir");
            exit(1);
        }
        
        struct TreeNode *pre_ptr = NULL;
        struct TreeNode *curr_ptr;
        struct dirent *entry_ptr;
        entry_ptr = readdir(d_ptr);

        while(entry_ptr != NULL) {
            // skip . and ..
            if((strcmp(entry_ptr->d_name, ".") == 0) | \
               (strcmp(entry_ptr->d_name, "..") == 0)) {
                entry_ptr = readdir(d_ptr);
                continue;
            }
            // set new_name and new_path
            char *new_name = entry_ptr->d_name;
            char new_path[(strlen(path)+strlen(new_name)+2)];
            strcpy(new_path, path);
            strcat(new_path, "/");
            strcat(new_path, new_name);
            fprintf(stderr, "newname: %s, newpath: %s\n", new_name, new_path);

            curr_ptr = generate_ftree_helper(new_name, new_path);
            if (tnode->contents == NULL) {
                tnode->contents = curr_ptr; 
            } else {
                pre_ptr->next = curr_ptr;
                printf("pre %s, cur %s\n", pre_ptr->fname, curr_ptr->fname);
                if(pre_ptr->contents == NULL) {
                    printf("!\n");
                }
                if(curr_ptr->contents == NULL) {
                    printf("!!\n");
                }
            }

            pre_ptr = curr_ptr;
            entry_ptr = readdir(d_ptr);
        }
        // curr_ptr->next = NULL;
        closedir(d_ptr);
    }
    return tnode;
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
    if((root->type == '-') | (root->type == 'l')) { // a file
        printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
    } else { // a directory
        printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
        struct TreeNode *curr_ptr = root->contents;
        depth++;
        while(curr_ptr != NULL) {
            print_ftree(curr_ptr);
            // fprintf(stderr, "cur: %s\n", curr_ptr->fname);
            curr_ptr = curr_ptr->next;
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

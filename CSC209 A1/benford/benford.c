#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "benford_helpers.h"

/*
 * The only print statement that you may use in your main function is the following:
 * - printf("%ds: %d\n")
 *
 */
int main(int argc, char **argv) {

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "benford position [datafile]\n");
        return 1;
    }
    // TODO: Implement.
    int tmp;
    int arr[BASE];
    for(int i = 0 ; i < BASE ; i++){
    	arr[i] = 0;
    }
    int count = 0;
    // for(int i = 0 ; i < BASE ; i++){
    // 	printf("%ds: %d\n", i, arr[i]);
    // }
    // printf("%d\n", atoi(argv[1]));
    while (scanf("%d", &tmp) != EOF){
    	// printf("%d\n", tmp);
    	// for(int i = 0 ; i < BASE ; i++){
	    // 	printf("%d\n", arr[i]);
	    // }
    	add_to_tally(tmp, atoi(argv[1]), arr);
    	// for(int i = 0 ; i < BASE ; i++){
	    // 	printf("%ds: %d\n", i, arr[i]);
	    // }
    }
    for(int i = 0 ; i < BASE ; i++){
	    printf("%ds: %d\n", i, arr[i]);
	}
}

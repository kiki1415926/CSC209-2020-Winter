#include <stdio.h>
#include <stdlib.h>

#include "life2D_helpers.h"


int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, "Usage: life2D rows cols states\n");
        return 1;
    }

    // TODO: Implement.
    int life;
    int num_rows = atoi(argv[1]);
    int num_cols = atoi(argv[2]);
    int board[num_rows * num_cols-1];
    int count = 0;
    int state = atoi(argv[3]);
    while (scanf("%d", &life) != EOF) {
    	board[count] = life;
    	count++;
    }
    print_state(board, num_rows, num_cols);
    for (int i = 1; i < state; i++) {
    	update_state(board, num_rows, num_cols);
    	print_state(board, num_rows, num_cols);
    }
    return 0;
}

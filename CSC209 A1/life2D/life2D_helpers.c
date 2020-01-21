#include <stdio.h>
#include <stdlib.h>


void print_state(int *board, int num_rows, int num_cols) {
    for (int i = 0; i < num_rows * num_cols; i++) {
        printf("%d", board[i]);
        if (((i + 1) % num_cols) == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

int check_neighbour(int *board, int num_cols, int i){
	return board[i - 1 - num_cols] + board[i - 1] + board[i - 1 + num_cols] 
	+ board[i - num_cols] + board[i + num_cols] + board[i + 1 - num_cols] + 
	board[i + 1] + board[i + 1 + num_cols];
}

void update_state(int *board, int num_rows, int num_cols) {
	// printf("debug: %d\n", check_neighbour(board, num_cols, 10));
    // TODO: Implement.
    int backup_board[num_rows * num_cols - 1];
    for (int i = 0; i < num_rows * num_cols; i++) {
    	backup_board[i] = board[i];
    }
    for (int i = 0; i < num_rows * num_cols; i++) {
    	// printf("%d\n", i);
    	if(i % num_cols == 0 || (i + 1) % num_cols == 0 || i < num_cols || (i >= (num_rows-1) * num_cols)){
    		// printf("no_change\n");
    		continue;
    	}else if (board[i] == 1){
    		// if(i == 10){
    		// 	printf("%d\n", check_neighbour(board, num_cols, i));
    		// }
    		if (check_neighbour(backup_board, num_cols, i) < 2 || check_neighbour(backup_board, num_cols, i) > 3){
    			board[i] = 0;
    		}
    	}else{
    		if (check_neighbour(backup_board, num_cols, i) == 2 || check_neighbour(backup_board, num_cols, i) == 3){
    			board[i] = 1;
    		}
    	}
    }
    return;
}

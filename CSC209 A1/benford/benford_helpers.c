#include <stdio.h>
#include <math.h>

#include "benford_helpers.h"

int count_digits(int num) {
    // TODO: Implement.
    int count = 0;
    while (pow(BASE, count) <= num){
    	count++;
    }
    return count;
}

int get_ith_from_right(int num, int i) {
    // TODO: Implement.
    int count = 0;
    int result = num;
    int mode = 0;
    while (count <= i){
    	mode = result % BASE;
    	result = result / BASE;
    	count++;
    }
    return mode;
}

int get_ith_from_left(int num, int i) {
    // TODO: Implement.
    int len_num = count_digits(num);
    // printf("len_num: %d\n", len_num);
    i = len_num - i - 1;
    // printf("i: %d\n", i);
    return get_ith_from_right(num, i);;
}

void add_to_tally(int num, int i, int *tally) {
    // TODO: Implement.
    int specific_pos = get_ith_from_left(num, i);
    int count = 0;
    // printf("specific_pos: %d\n", specific_pos);
    while( count < specific_pos ){
    	tally++;
    	count++;
    }
    *tally += 1;
    return;
}
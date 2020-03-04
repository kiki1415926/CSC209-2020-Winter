#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "point.h"
#include "serial_closest.h"
#include "utilities_closest.h"


/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount) {
    if (n < 4 || pdmax == 0) return closest_serial(p, n);

    // Step 2
	int mid = n / 2;
    struct Point mid_point = p[mid];
    
    // 3. Create two child processes to to solve the left half and right half of the problem
    int fd[2][2];
    for (int i = 0; i < 2; i++) {
    	if (pipe(fd[i]) == -1) {
	        perror("pipe");
	        exit(1);
	    }
	    pid_t ret = fork();
	    if (ret > 0) {
	    	close(fd[i][1]);
	    } else if (ret == 0) {
	    	close(fd[i][0]);
	    	int pcount = 0;
	    	double min_dist;
	    	if (i == 0)
	    		min_dist = closest_parallel(p, mid, pdmax-1, &pcount);
	    	else
	    		min_dist = closest_parallel(p+mid, n-mid, pdmax-1, &pcount);
	    	if (write(fd[i][1], &min_dist, sizeof(double)) == -1) {
	            perror("write");
	            exit(1);
	        }
	        close(fd[i][1]);
	        exit(pcount);
	    } else {
	    	perror("fork");
			exit(1);
	    }
    }

    // 4. Wait for both child processes to complete (each process should have at most two child processes).
    for (int i = 0; i < 2; i++) {
    	pid_t pid;
    	int status;
        if ( (pid = wait(&status)) == -1) {
            perror("wait");
        } else {
            if (WIFEXITED(status)) {
                (*pcount) += WEXITSTATUS(status) + 1;
            } else {
                fprintf(stderr, "Child terminated abnormally\n");
                exit(1);
            }
        }
    }

    // 5. Read from the two pipes to retrieve the results from the two child processes.
    double ds[2];
    for (int i = 0; i < 2; i++) {
    	if (read(fd[i][0], ds+i, sizeof(double)) == -1) {
	        perror("read");
	        exit(1);
	    }
	    close(fd[i][0]);
    }

    // 6. Find the distance between the closest pair of points, with distance is smaller than d
    double d = min(ds[0], ds[1]);
    struct Point strip[n];
    int j = 0;
    for (int i = 0; i < n; i++) {
        if (abs(p[i].x - mid_point.x) < d) {
            strip[j] = p[i], j++;
        }
    }
    return min(d, strip_closest(strip, j, d));
}


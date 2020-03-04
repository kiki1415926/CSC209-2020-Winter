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
    if (n < 4 || pdmax == 0) {
    	*pcount = 0;
    	return closest_serial(p, n);
    }

	int mid = n / 2;
    struct Point mid_point = p[mid];

    // left: idx 0 ~ mid-1
    // right: idx mid ~ end
    
    // LEFT
    int lfd[2];
    if (pipe(lfd) == -1) {
        perror("pipe");
        exit(1);
    }
    pid_t lret = fork();
    if (lret > 0) {
    	close(lfd[1]);
    	// parent process do nothing
    } else if (lret == 0) {
    	close(lfd[0]);
    	int lpcount = 0;
    	double lmin_dist = closest_parallel(p, mid, pdmax-1, &lpcount);
    	if (write(lfd[1], &lmin_dist, sizeof(double)) == -1) {
            perror("write");
            exit(1);
        }
        close(lfd[1]);
        exit(lpcount);
    } else {
    	perror("fork");
		exit(1);
    }

    // RIGHT
    int rfd[2];
    if (pipe(rfd) == -1) {
        perror("pipe");
        exit(1);
    }
    pid_t rret = fork();
    if (rret > 0) {
    	close(rfd[1]);
    	// parent process do nothing
    } else if (rret == 0) {
    	close(rfd[0]);
    	int rpcount = 0;
    	double rmin_dist = closest_parallel(p+mid, n-mid, pdmax-1, &rpcount);
    	if (write(rfd[1], &rmin_dist, sizeof(double)) == -1) {
            perror("write");
            exit(1);
        }
        close(rfd[1]);
        exit(rpcount);
    } else {
    	perror("fork");
		exit(1);
    }

    // wait
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

    // Read from the two pipes to retrieve the results from the two child processes.
    double dl;
	if (read(lfd[0], &dl, sizeof(double)) == -1) {
        perror("read");
        exit(1);
    }
    close(lfd[0]);
    double dr;
    if (read(rfd[0], &dr, sizeof(double)) == -1) {
        perror("read");
        exit(1);
    }
    close(rfd[0]);

    // Find the distance between the closest pair of points, with distance is smaller than d
    double d = min(dl, dr);
    struct Point *strip = malloc(sizeof(struct Point) * n);
    if (strip == NULL) {
        perror("malloc");
        exit(1);
    }
    int j = 0;
    for (int i = 0; i < n; i++) {
        if (abs(p[i].x - mid_point.x) < d) {
            strip[j] = p[i], j++;
        }
    }
    double new_min = min(d, strip_closest(strip, j, d));
    free(strip);

    return new_min;
}


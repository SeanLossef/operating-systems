#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#ifdef NO_PARALLEL
#define PARALLEL 0
#else
#define PARALLEL 1
#endif

// Data Struct -- Used for game information between threads
typedef struct {
	int *max_squares; // Points to global int

	char ***dead_end; // Global array of dead end grids
	int *dead_end_count; // Global length of ***dead_end

	pthread_mutex_t lock; // Locks the above global variables

	char **grid; // Grid passed down from parent thread
	int m, n, sonny_x, sonny_y, move, x;
} data;

// Frees memory for a grid object
void free_grid(char **grid, int n) {
	for (int i = 0; i < n; i++)
		free(grid[i]);
	free(grid);
}

// Returns true if (x,y) is a valid move not yet visited
int can_move(char **grid, int m, int n, int x, int y) {
	if (x < 0 || y < 0 || x > n - 1 || y > m - 1)
		return 0;
	return grid[x][y] == '.';
}

/* Returns list of booleans where sonny can move
 *   . 1 . 2 .
 *   0 . . . 3
 *   . .<S>. .
 *   7 . . . 4
 *   . 6 . 5 .
 */
int* moves(char **grid, int m, int n, int x, int y) {
	int *dirs = malloc(8 * sizeof(int));

	dirs[0] = can_move(grid, m, n, x - 2, y - 1);
	dirs[1] = can_move(grid, m, n, x - 1, y - 2);
	dirs[2] = can_move(grid, m, n, x + 1, y - 2);
	dirs[3] = can_move(grid, m, n, x + 2, y - 1);
	dirs[4] = can_move(grid, m, n, x + 2, y + 1);
	dirs[5] = can_move(grid, m, n, x + 1, y + 2);
	dirs[6] = can_move(grid, m, n, x - 1, y + 2);
	dirs[7] = can_move(grid, m, n, x - 2, y + 1);

	return dirs;
}

int get_dir(int *dirs) {
	for (int i = 0; i < 8; i++)
		if (dirs[i])
			return i;
	return 0;
}

void make_move(char **grid, int dir, int *x, int *y) {
	if (dir == 0) { *x -= 2; *y -= 1; }
	if (dir == 1) { *x -= 1; *y -= 2; }
	if (dir == 2) { *x += 1; *y -= 2; }
	if (dir == 3) { *x += 2; *y -= 1; }
	if (dir == 4) { *x += 2; *y += 1; }
	if (dir == 5) { *x += 1; *y += 2; }
	if (dir == 6) { *x -= 1; *y += 2; }
	if (dir == 7) { *x -= 2; *y += 1; }

	grid[*x][*y] = 'S';
}

// Returns number of valid moves given dirs array
int num_moves(int *dirs) {
	int count = 0;
	for (int i = 0; i < 8; i++)
		if (dirs[i])
			count++;
	return count;
}

// Prints grid
void print_grid(char **grid, int m, int n) {
	for (int i = 0; i < m; i++) {
		printf("THREAD %ld: ", (long int) pthread_self());
		if (i == 0)
			printf("> ");
		else
			printf("  ");
		for (int j = 0; j < n; j++) {
			printf("%c", grid[j][i]);
		}
		printf("\n");
	}
}

// Returns copy of grid
char** cpy_grid(char **src, int m, int n) {
	char **dest = malloc(n * sizeof(char *));
	for (int i = 0; i < n; i++) {
		dest[i] = malloc(m * sizeof(char));
		memcpy(dest[i], src[i], m);
	}
	return dest;
}

// Adds grid to array of grids
void add_grid(char ***array, char **grid, int count) {
	array[count] = grid;
}

// Called when game tree splits to new thread
void* pthread_handler(void *arg) {
	data *d = (data*) arg;
	
	while (1) {
		int *dirs = moves(d->grid, d->m, d->n, d->sonny_x, d->sonny_y);
		int nmoves = num_moves(dirs);

		if (nmoves > 1) {
			printf("THREAD %ld: %d moves possible after move #%d; creating threads...\n", (long int) pthread_self(), nmoves, d->move);

			pthread_t tid[nmoves];
			

			// Create threads
			for (int i = 0; i < nmoves; i++) {
				data d_child;
				d_child.max_squares = d->max_squares;
				d_child.dead_end = d->dead_end;
				d_child.dead_end_count = d->dead_end_count;
				d_child.grid = cpy_grid(d->grid, d->m, d->n);
				d_child.m = d->m;
				d_child.n = d->n;
				d_child.sonny_x = d->sonny_x;
				d_child.sonny_y = d->sonny_y;
				d_child.move = d->move + 1;
				d_child.lock = d->lock;
				d_child.x = d->x;

				int dir = get_dir(dirs);
				dirs[dir] = 0;
				make_move(d_child.grid, dir, &(d_child.sonny_x), &(d_child.sonny_y));

				pthread_create(&tid[i], NULL, pthread_handler, &d_child);
				if (PARALLEL == 0) {
					int *ret = malloc(0);
					free(ret);
					pthread_join(tid[i], (void**) &ret);
					printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", (long int) pthread_self(), (long int) tid[i], *ret);
					free(ret);
				}
				
			}

			// Wait for all threads to complete
			if (PARALLEL != 0) {
				for (int i = 0; i < nmoves; i++) {
					int *ret = NULL;
					pthread_join(tid[i], (void**) &ret);
					printf("THREAD %ld: Thread [%ld] joined (returned %d)\n", (long int) pthread_self(), (long int) tid[i], *ret);
					free(ret);
				}
			}


			free_grid(d->grid, d->n);
			free(dirs);

			int *ret = malloc(sizeof(int*));
			*ret = 0;
			return ret;
		}
		if (nmoves == 1) {
			int dir = get_dir(dirs);
			make_move(d->grid, dir, &(d->sonny_x), &(d->sonny_y));
			d->move++;
			free(dirs);
		}
		if (nmoves == 0) {
			pthread_mutex_lock(&d->lock);

			if (d->move == d->m * d->n)
				printf("THREAD %ld: Sonny found a full knight's tour!\n", (long int) pthread_self());
			else {
				if (d->move >= d->x) {
					add_grid(d->dead_end, cpy_grid(d->grid, d->m, d->n), *(d->dead_end_count));
					*(d->dead_end_count) = *(d->dead_end_count) + 1;
				}

				printf("THREAD %ld: Dead end after move #%d\n", (long int) pthread_self(), d->move);
			}

			if (d->move > *(d->max_squares))
				*(d->max_squares) = d->move;

			pthread_mutex_unlock(&d->lock);

			int *ret = malloc(sizeof(int*));
			*ret = d->move;
			free(dirs);
			free_grid(d->grid, d->n);
			pthread_exit(ret);
		}
	}
}

int main(int argc, char** argv) {
	setvbuf( stdout, NULL, _IONBF, 0 );

	// Validate input
	if (argc != 3 && argc != 4) {
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}

	int m = atoi(argv[1]);
	int n = atoi(argv[2]);
	int x = 8;
	if (argc == 4)
		x = atoi(argv[3]);

	if (m <= 2 || n <= 2) {
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}

	if (x < 0 || x > m * n) {
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<x>]\n");
		return EXIT_FAILURE;
	}

	// Maximum number of possible dead ends given n, m, x
	int max_dead_ends = 1;
	for (int i = m*n; i > x; i--)
		max_dead_ends *= i;

	// Initialize problem variables
	int *max_squares = malloc(sizeof(int));
	*max_squares = 0;
	int *dead_end_count = malloc(sizeof(int));
	*dead_end_count = 0;
	char ***dead_end = calloc(max_dead_ends, sizeof(char**));

	// Initialize Grid
	char **grid = malloc(n * sizeof(char *));
	for (int i = 0; i < n; i++)
		grid[i] = malloc(m * sizeof(char));

	for (int i = 0; i < n; i++)
		for (int j = 0; j < m; j++)
			grid[i][j] = '.';
	grid[0][0] = 'S';

	// Start Solving
	printf("THREAD %ld: Solving Sonny's knight's tour problem for a %dx%d board\n", (long int) pthread_self(), m, n);

	data d;
	d.max_squares = max_squares;
	d.dead_end = dead_end;
	d.dead_end_count = dead_end_count;
	d.grid = grid;
	d.m = m;
	d.n = n;
	d.sonny_x = 0;
	d.sonny_y = 0;
	d.move = 1;
	d.x = x;

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&d.lock, &attr);

	int *ret;
	pthread_t worker;
	pthread_create(&worker, NULL, pthread_handler, &d);
	pthread_join(worker, (void**)&ret);
	free(ret);

	printf("THREAD %ld: Best solution(s) found visit %d squares (out of %d)\n", (long int) pthread_self(), *(d.max_squares), m * n);
	printf("THREAD %ld: Dead end boards:\n", (long int) pthread_self());
	for (int i = 0; i < *dead_end_count; i++)
		print_grid(dead_end[i], m, n);

	// Free Memory
	for (int i = 0; i < max_dead_ends; i++)
		if (dead_end[i] != NULL)
			free_grid(dead_end[i], n);
	free(dead_end);
	free(max_squares);
	free(dead_end_count);

	return EXIT_SUCCESS;
}
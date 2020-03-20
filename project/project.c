#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>

#define MAXPROCESS 100


// A structure to represent a cpu burst
typedef struct Burst Burst;
struct Burst {
	int cpu_time;      // Initial CPU burst time
	int cpu_remaining; // CPU burst time remaining, updated when running
	int io_time;
	int io_remaining;
};

// A structure to represent a process
typedef struct Process Process;
struct Process {
	char pid;
	int arrival;       // Arrival time in ms
	int burst_count;   // Number of bursts there are
	int current_burst; // Index of burst currently in progress
	Burst* bursts;     // Array of all bursts
};





// A structure to represent a queue
typedef struct Queue Queue;
struct Queue 
{ 
    int front, rear, size; 
    unsigned capacity; 
    Process** array; 
}; 
  
// function to create a queue of given capacity.  
// It initializes size of queue as 0 
Queue* createQueue(unsigned capacity) 
{ 
    struct Queue* queue = (Queue*) malloc(sizeof(Queue)); 
    queue->capacity = capacity; 
    queue->front = queue->size = 0;  
    queue->rear = capacity - 1;  // This is important, see the push 
    queue->array = (Process**) malloc(queue->capacity * sizeof(Process*)); 
    return queue; 
} 
  
// Queue is full when size becomes equal to the capacity  
int isFull(Queue* queue) 
{  return (queue->size == queue->capacity);  } 
  
// Queue is empty when size is 0 
int isEmpty(Queue* queue) 
{  return (queue->size == 0); } 
  
// Function to add an item to the queue.   
// It changes rear and size 
void push(Queue* queue, Process* item) 
{ 
    if (isFull(queue)) 
        return; 
    queue->rear = (queue->rear + 1)%queue->capacity; 
    queue->array[queue->rear] = item; 
    queue->size = queue->size + 1; 
} 
  
// Function to remove an item from queue.  
// It changes front and size 
Process* pop(Queue* queue) 
{ 
    if (isEmpty(queue)) 
        return NULL; 
    Process* item = queue->array[queue->front]; 
    queue->front = (queue->front + 1)%queue->capacity; 
    queue->size = queue->size - 1; 
    return item; 
} 
  
// Function to get front of queue 
Process* front(Queue* queue) 
{ 
    if (isEmpty(queue)) 
        return NULL; 
    return queue->array[queue->front]; 
} 
  
// Function to get rear of queue 
Process* rear(Queue* queue) 
{ 
    if (isEmpty(queue)) 
        return NULL; 
    return queue->array[queue->rear]; 
}




void list_push(Process** list, Process* item) {
	list[0] = item;
	// for (int i = 0; i < MAXPROCESS; i++) {
	// 	if (list[i] == NULL) {
	// 		list[i] = item;
	// 		return;
	// 	}
	// }
}






// Exponential Random Function
double erand(double lambda) {
	double r = drand48();
    return -log( r ) / lambda;
}

void print_process(Process p) {
	printf("Process:\n\tpid: %c\n\tarrival: %dms\n\tcpu_bursts: %d\n", p.pid, p.arrival, p.burst_count);
	for (int i = 0; i < p.burst_count; i++) {
		printf("\t\tCPU Burst: %dms, IO Burst: %dms\n", p.bursts[i].cpu_time, p.bursts[i].io_time);
	}
}

void print_event(int t, char* message, Queue* queue) {
	if (isEmpty(queue))
		printf("time %dms: %s [Q <empty>]\n", t, message);
	else {
		char *queue_str = calloc(MAXPROCESS, 1);
		for (int i = 0; i < queue->size; i++)
			sprintf(queue_str, "%s %c", queue_str, queue->array[(queue->front + i) % queue->capacity]->pid);

		printf("time %dms: %s [Q%s]\n", t, message, queue_str);
		free(queue_str);
	}
}

int main(int argc, char** argv) {
	if (argc != 8 && argc != 9) {
		fprintf(stderr, "Usage: python3 project1.py seed lambda u_bound n t_cs alpha t_slice [rr_add]");
		return 1;
	}

	char buffer[256];

	// Get command line arguments
	long seed     = atoi(argv[1]);
	double lambda = strtod(argv[2], NULL);
	int u_bound   = atoi(argv[3]);
	int n         = atoi(argv[4]);
	int t_cs      = atoi(argv[5]);
	double alpha  = strtod(argv[6], NULL);
	int t_slice   = atoi(argv[7]);
	// char* rr_add  = argv[8] if argc == 9 else "END"

	//printf("seed: %ld\nlambda: %lf\nu_bound: %d\nn: %d\nt_cs: %d\nalpha: %lf\nt_slice: %d\n", seed, lambda, u_bound, n, t_cs, alpha, t_slice);

	srand48(seed);

	// Generate Pseudo-Random Processes
	Process* processes = malloc(n * sizeof(Process));
	for (int i = 0; i < n; i++) {
		processes[i].pid = 'A'+i;
		processes[i].arrival = floor(erand(lambda));
		processes[i].burst_count = ceil(drand48() * 100);
		processes[i].current_burst = 0;
		processes[i].bursts = malloc(processes[i].burst_count * sizeof(Burst));
		for (int j = 0; j < processes[i].burst_count; j++) {
			processes[i].bursts[j].cpu_time = processes[i].bursts[j].cpu_remaining = ceil(erand(lambda));
			if (j == processes[i].burst_count - 1)
				processes[i].bursts[j].io_time = processes[i].bursts[j].io_remaining = 0;
			else
				processes[i].bursts[j].io_time = processes[i].bursts[j].io_remaining = ceil(erand(lambda));
		}
		print_process(processes[i]);
	}

	// FCFS ALGORITHM
	Queue* ready_queue = createQueue(MAXPROCESS);
	Process* running = NULL;
	Process** blocked = calloc(MAXPROCESS, sizeof(Process*));

	int t = 0;

	// Print new processes
	for (int i = 0; i < n; i++)
		printf("Process %c [NEW] (arrival time %d ms) %d CPU bursts\n", processes[i].pid, processes[i].arrival, processes[i].burst_count);
	
	print_event(0, "Simulator started for FCFS", ready_queue);
	
	for (; t < 3000; t++) {

		// Loop through blocked processes
		for (int i = 0; i < MAXPROCESS; i++) {
			if (blocked[i] != NULL) {
				if (blocked[i]->bursts[blocked[i]->current_burst].io_remaining == 0) {
					blocked[i]->current_burst++;
					push(ready_queue, blocked[i]);
					sprintf(buffer, "Process %c completed I/O; added to ready queue", blocked[i]->pid);
					print_event(t, buffer, ready_queue);
					blocked[i] = NULL;
				} else {
					// Decrement IO Blocked process timer
					blocked[i]->bursts[blocked[i]->current_burst].io_remaining--;
				}
			}
		}

		// Increment running process
		if (running != NULL) {
			
			// CPU Burst is complete
			if (running->bursts[running->current_burst].cpu_remaining == -1) {

				sprintf(buffer, "Process %c completed a CPU burst; %d bursts to go", running->pid, running->burst_count - running->current_burst - 1);
				print_event(t, buffer, ready_queue);

				// Add to IO blocked list
				if (running->bursts[running->current_burst].io_time > 0) {
					sprintf(buffer, "Process %c switching out of CPU; will block on I/O until time %dms", running->pid, t + (running->bursts[running->current_burst].io_time));
					print_event(t, buffer, ready_queue);

					list_push(blocked, running);
				} else {
					running->current_burst++;
				}
				running = NULL;
			} else {
				// Decrement amount of CPU time running process has left
				running->bursts[running->current_burst].cpu_remaining--;
			}
		}

		// Start running process if one is not currently
		if (isEmpty(ready_queue) == 0 && running == NULL) {
			running = pop(ready_queue);
			sprintf(buffer, "Process %c started using the CPU for %dms burst", running->pid, running->bursts[running->current_burst].cpu_time);
			print_event(t+1, buffer, ready_queue);
			continue;
		}

		for (int i = 0; i < n; i++) {
			// Add process to queue if now is its arrival time
			if (t == processes[i].arrival) {
				push(ready_queue, &processes[i]);
				sprintf(buffer, "Process %c arrived; added to ready queue", processes[i].pid);
				print_event(t, buffer, ready_queue);
			}
		}
	}

	return 0;
}
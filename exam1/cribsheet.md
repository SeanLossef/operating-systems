## Pipes
```
int fd[2];
int ret = pipe(fd);
```
Returns 0 on success, -1 on failure

### Preventing holding reads
When child closes write pipe, it adds EOF and doesn't allow hang.
```c
/* fd table:     [each process has its own fd table]
[PARENT]                                               [CHILD]
0: stdin                                               0: stdin
1: stdout                                              1: stdout
2: stderr                        +--------+            2: stderr
3: pipefd[0] <======READ======== | buffer |            3: p[0]
4: pipefd[1]                     | buffer | <==WRITE== 4: p[1]
                                 +--------*/
if (pid == 0) {
  close(pipefd[0]);
  int bytes_written = write(pipefd[1], "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
  close(pipefd[1]);
} else {
  close(pipefd[1]);
  // Read
  close(pipefd[0]);
}
```

## File Descriptors

`int dup(int oldfd);`  
Copies oldfd into the lowest-numbered unused descriptor.  
If successful, new and old fd's can be used interchangeably, else return -1.  

`int dup2(int oldfd, int newfd);`  
Copies oldfd into newfd, returns new fd
If the descriptor newfd was previously open, it is silently closed before being reused.  
If oldfd is not a valid file descriptor, then the call fails, and newfd is not closed.  
If oldfd is a valid file descriptor, and newfd has the same value as oldfd, then dup2() does
nothing, and returns newfd.  

`int open(const char *pathname, int flags);`  
Returns fd, which is lowest-numbered fd not currently open for the process.  
By default new fd is set to remain open across exec().  

`ssize_t write(int fd, buffer, count);`
On success returns number of bytes written, -1 on error.

man 2 open
man dup2
close

## Scheduling Algorithms
Probability everybody is in I/O for `n` processes and each spends `p` percent in I/O is `p^n`  
TURNAROUND TIME  =  WAIT TIME  +  CPU BURST TIME  +  OVERHEAD (context switches)

  RUNNING STATE: process is actively using the CPU  
  READY STATE: process is ready to use the CPU  
  WAITING STATE: process is waiting for I/O operation(s) to complete  

### Priority Scheduling  
Each process assigned a priority based on  
- CPU burst times (SJF/SRT) <= estimated
- ratio of CPU to I/O activity
- system resource usage

If preemptive, arriving processes of higher priority will be context-switched into CPU  
Non-preemptive allows process to hold CPU indefinitely
- Aging: to prevent starvation
- Increase priority of process in ready queue over time

### Round Robin
FCFS with a fixed time limit on each CPU burst
- Time quantum: finish within or preempted
- Heuristic that 80% of CPU burst times should be less then timeslice
```
Apply the RR algorithm to the processes listed below using a timeslice of 3ms

 pid   CPU burst times     arrival times
 P1      20 ms                 0
 P2       5 ms                 0
 P3       2 ms                 2 ms
 P4      10 ms                 4 ms

QUEUE: 

   RR (with timeslice of 3ms)
    |
  P1>XXXp    XXXp    XXXp  XXXp  XXXpXXXXX.  <== 5 preemptions
  P2>   XXXp       XX.                       <== 1 preemption
  P3| >    XX.                               <== 0 preemptions
  P4|   >       XXXp    XXXp  XXXp  X.       <== 3 preemptions
    +--------------------------------------------> time
              111111111122222222223333333333
    0123456789012345678901234567890123456789

 What is the wait time and turnaround time for each process?

 P1 has 17ms of wait time; P1 has 37ms of turnaround time
 P2 has 11ms of wait time; P2 has 16ms of turnaround time
 P3 has 4ms of wait time; P3 has 6ms of turnaround time
 P4 has 18ms of wait time; P4 has 28ms of turnaround time
```
```
===================================================================
ALGORITHM   PREEMPTION?     ADVANTAGE(S)           DISADVANTAGE(S)

 FCFS       non-preemptive  easy to implement      long wait times
                            minimal overhead       long turnaround times
                            no starvation

 SJF        non-preemptive  optimal (fastest)      starvation
                             (least average        requires us to predict
                               wait time)            CPU burst times

 SRT        preemptive                             starvation
                                                   requires us to predict
                                                     CPU burst times

 Priority   non-preemptive  finer control over     starvation
             or preemptive    process order

 Priority   non-preemptive  no starvation          but we still have
  w Aging    or preemptive                          long wait times for
   (PWA)                                             low-priority processes

 Round      preemptive      no starvation          longer average wait times
  Robin      based on       fairness               increased overhead
   (RR)       timeslice                              (more context switches)
               expiration                          strong dependency on
                                                     timeslice selection

===================================================================
```

### Exponential Averaging

tau   -- estimated burst time  
t     -- actual burst time  
alpha -- constant in the range [0.0,1.0), often 0.5 or higher

tau i+1   =  alpha x t i   +  (1-alpha) x tau i


## Exec
`execl("/bin/ls", "ls", NULL);`  
Replaces process memory if exec doesn't fail

`strlen("Hello") = 5`
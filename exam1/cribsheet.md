## Pipes
```
int fd[2];
int ret = pipe(fd);
```
Returns 0 on success, -1 on failure

### Preventing handing reads
When child closes write pipe, it adds EOF and doesn't allow hang.
```
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

man 2 open
man dup2

## Scheduling Algorithms
Probability everybody is in I/O for `n` processes and each spends `p` percent in I/O is `p^n`

First come first serve

Shortest job first
min wait time
low turnarounf for interactive processes

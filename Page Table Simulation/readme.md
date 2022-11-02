The following code simulates a page table and given a sequence of memory
accesses and whether each access was read or write, outputs the number
of page faults, the number of reads from the disk and the number of writes to the disk. The
specifications are given below.

### Details
1. 5 strategies - OPT, FIFO, CLOCK, LRU, and RANDOM are implemented
2. The program will take input a trace file named trace.in, an integer denoting the
number of frames available, and a string which would denote the strategy. For instance, if the executable is named foo, then an example command that should work
is ./foo trace.in 100 OPT
3. The trace file is a list of virtual memory addresses followed by letter "R" or "W", which
would indicate whether the memory access was a read or write. Each line of the trace file
would have one such memory access.
4. The string for the strategies would be named as following: OPT, FIFO, CLOCK, LRU,
and RANDOM.
5. The size of one frame is 4KB, and assume that the physical memory can hold number of
frames that is given as an integer input (maximum 1000).

# Operating-Systems
## Memory Virtualization
This project is about memory virtualization using page tables. Single
level page tables are implemented, along with an API to simulate the memory, given a 200 MB RAM, to be used by OS to create and
manage user-processes. The first portion of the RAM will be used by the OS and the remaining will be assigned to
processes by OS. The OS supports creation, stopping and forking of processes (only the memory management part of it). Each
process has 4 MB of virtual memory, and the page size is set to be 4KB. The OS flexibly maps the virtual pages of processes to the physical frames.



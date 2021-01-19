# ECE-357-Operating-Systems
Problem sets for ECE-357: Computer Operating Systems

## Project Descriptions
- Filesystem: Program that uses directory exploration library functions such as `readdir` to produce a recursive
directory listing which is similar to the output of `find /starting_directory -ls`

- Minicat: Implementation of the UNIX cat command for concatenating and copying files.

- Pipes: Upgraded implementation of cat: Concatenates and copies files using pipes rather than standard output.

- Semaphores: Implementation of four semaphore operations utilizing spinlock mutexes: initialization, decrementing, blocking, and waking. 

- Shell: Implementation of UNIX shell: launches one program at a time, with arguments. Reports exit status and resource usage statistics.

- Smear: Utilizes `mmap` system calls to search for and replace strings in files.

- Strace: Uses the `strace` UNIX command to analyze the system calls of a simple C program. Analysis of system calls and exit codes included.


## Tech Stack
- C Programming Language
- Assembly


## Course Description
Theory and implementation of modern computer operating systems. Message based and multiprocessor kernels. Networking and interprocess communication. Security, auditing and authentication. Device drivers, interrupt handling, task switching, virtual memory, memory management, scheduling, synchronization and locking. File systems, resource allocation and management. Real-time, fault-tolerant and high security operating systems. User environment and interface issues. Projects in operating system design and programming, case studies.

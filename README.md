# Operating Systems Lab – Assignment 5
## Process Synchronization using xv6

**Roll Number:** 2403MC04  
**Name:** Leon Joel  

## Overview

This assignment implements three classical process synchronization problems in xv6:

1. Peterson's Algorithm
2. Producer-Consumer Problem
3. Readers-Writers Problem

The implementations use shared memory and lightweight semaphores added to xv6 where required.

## Question 1 – Peterson's Algorithm

Peterson's algorithm is implemented for a parent process and its child process. A shared memory page is used to store `flag[2]`, `turn`, and `shared_counter`.

The two processes execute the critical section for 10 iterations each. Mutual exclusion is achieved using only the Peterson algorithm variables. The final value of the shared counter is expected to be 20.

A `shm_get()` system call was added to xv6 to create a shared page and map the same physical page into the child after `fork()`.

## Question 2 – Producer-Consumer

A fixed circular buffer of size 5 is shared between a producer and consumer process.

Three semaphores are used:

- `empty` – number of available buffer slots
- `full` – number of occupied slots
- `mutex` – protects access to the shared buffer

The producer generates items 1 through 20 and the consumer removes them in order. The output demonstrates the producer blocking when the buffer becomes full.

The semaphore implementation uses xv6's `sleep()` and `wakeup()` mechanisms.

## Question 3 – Readers-Writers

The readers-writers problem is implemented using three reader processes and two writer processes.

`read_count` is protected using a mutex, while a separate semaphore provides exclusive access to `shared_data` for writers. Multiple readers can access the data concurrently.

A turnstile semaphore is used to prevent writers from being indefinitely starved by incoming readers.

## Files

- `peterson.c` – Peterson's algorithm
- `prodcons.c` – Producer-consumer implementation
- `readwrite.c` – Readers-writers implementation
- `kernel/semaphore.c` and `kernel/semaphore.h` – semaphore implementation
- `kernel/sysproc.c`, `kernel/syscall.c`, `kernel/syscall.h` – system call support
- `kernel/proc.c`, `kernel/proc.h`, `kernel/memlayout.h` – shared-memory support
- `Makefile` – build configuration

## Build and Run

The modified xv6 source was built using the RISC-V cross compiler and tested with QEMU.

```bash
make clean
make
make qemu CPUS=2
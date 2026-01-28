# CMPUT 379: Operating Systems Assignments

This repository contains assignments and resources for the CMPUT 379 Operating Systems course. The projects cover fundamental OS concepts including process management, inter-process communication (IPC), network programming, and multithreading synchronization.

## Repository Structure

The repository is organized into the following directories:

### [Assignment 1: Job Control Shell](./379/Assignment1)
Implementation of a custom shell that handles process creation and job control.
- **Key Features:**
  - Execution of commands using `fork()` and `exec()`.
  - Job control commands: `run`, `list`, `suspend`, `resume`, `terminate`.
  - Signal handling to manage child processes.
  - Monitoring process resources (CPU time, user time, system time).
- **Key File:** `a1jobs.c`

### [Assignment 2: SDN with FIFOs](./379/Assignment2)
Simulation of a Software Defined Networking (SDN) environment using Named Pipes (FIFOs).
- **Key Features:**
  - **Controller & Switches:** Implemented as separate processes communicating via FIFOs.
  - **I/O Multiplexing:** Uses `poll()` to handle input from multiple sources (user input, other switches, controller) without blocking.
  - **Packet Handling:** Simulates OPEN, QUERY, ADD, ACK, and RELAY packets.
- **Key File:** `a2sdn.cc` (or `a2sdn.c`)

### [Assignment 3: SDN with Sockets](./379/Assignment3)
Extension of the SDN simulation to support network communication using TCP Sockets.
- **Key Features:**
  - **Socket Programming:** Replaces FIFOs with TCP/IP sockets for communication between Controller and Switches.
  - **Distributed System:** Allows the Controller and Switches to run on different machines (simulated via localhost in the provided code).
  - **Protocol:** Implements the same SDN protocol as Assignment 2 but over a network layer.
- **Key File:** `a2sdn_2.cc` (compiles to `a2sdn`)

### [Assignment 4: Multithreading & Synchronization](./379/Assignment4)
A multithreaded application demonstrating resource sharing and deadlock prevention.
- **Key Features:**
  - **Thread Creation:** Uses `pthread` to create multiple task threads.
  - **Synchronization:** Implements mutexes to protect critical sections and shared resources.
  - **Deadlock Prevention:** Implements strategies to avoid deadlocks when multiple threads request resources.
  - **Monitoring:** A monitor thread tracks the status (WAIT, IDLE, RUN) of all tasks.
- **Key File:** `a4tasks.cc`

## Resources

- **[Course Notes](./untitled):** A collection of notes covering threads, processes, signals, IPC, and synchronization.
- **Readings:** `reading379.pdf` (located in the `379` directory).

## Compilation and Usage

Each assignment directory typically contains a `Makefile`. You can compile the projects using `make`.

```bash
cd 379/Assignment1
make
./a1jobs
```

*Note: Check individual Makefiles for specific targets and instructions.*

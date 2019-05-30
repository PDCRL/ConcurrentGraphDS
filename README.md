# ConcurrentGraphDS
This repository contains the implementation of a blocking & non-blocking based Concurrent Graph Data Structure.
The algorithm is described in technical report '**Maintaining Acyclicity of Concurrent Graph Objects**' which can be accessed here: https://arxiv.org/abs/1611.03947.

The subdirectory */data_structure* provides the source code for Graph data structure variants, as decribed below:
1. Sequential Graph data structure
2. Concurrent Coarse Lock Graph Data Structure
3. Concurrent Fine Lock Graph Data Structure without deletion of incoming edges of deleted vertices (Cycle detection using collect)
4. Concurrent Fine Lock Graph Data Structure with deletion of incoming edges of deleted vertices (Cycle detection using Reachability)
5. Concurrent Fine Lock Graph Data Structure without deletion of incoming edges of deleted vertices (Cycle detection using collect)
6. Concurrent Fine Lock Graph Data Structure with deletion of incoming edges of deleted vertices (Cycle detection using Reachability)

The subdirectory *Sequential* provides the source code for Graph data structure variants and Acyclicity, as decribed below:
1. Sequential Graph data-structure implemented using composition of list-based set.
2. Sequential Acyclic Graph data-structure implemented using composition of list-based set.

The subdirectory *coarse-lock* provides the source code for Graph data structure variants and Acyclicity, as decribed below:
1. Coarse-lock Graph data-structure implemented using composition of list-based set.
2. Coarse-lock Acyclic Graph data-structure implemented using composition of list-based set.

The subdirectory *HoH-locking* provides the source code for Graph data structure variants and Acyclicity, as decribed below:
1. HoH-locking Graph data-structure implemented using composition of list-based set.
2. HoH-locking Acyclic Graph data-structure implemented using composition of list-based set.

The subdirectory *Lazylist-lock* provides the source code for Graph data structure variants and Acyclicity, as decribed below:
1. lazylist-lock Graph data-structure implemented using composition of list-based set.
2. lazylist-lock Acyclic Graph data-structure implemented using composition of list-based set.

The subdirectory *non-blocking* provides the source code for Graph data structure variants and Acyclicity, as decribed below:
1. Lock-free Graph data-structure implemented using composition of list-based set.
2. Lock-free Acyclic Graph data-structure implemented using composition of list-based set.

To compile any source file, run the command:
`g++ -std=c++11 filename.cpp -lpthread -o filename.o -O3`

To run this binary file, use the following format:
`./filename numOfThreads numOfInitialVertices runningTimeInSecond`

Example:
1. For sequential program run: `./sequential 1 1000 20`
2. Whereas for a Concurrent program run: `./conc 10 1000 20`
(A concurrent program of 10 threads with 1000 initial vertices run for 20 seconds.)

You can vary the workload distribution of these operations inside each of these programs.

If you have any questions, please contact: cs15mtech01004@iith.ac.in , cs15resch11012@iith.ac.in

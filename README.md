# ConcurrentGraphDS
This repository contains the source code of the Concurrent Graph Data Structure of the paper https://arxiv.org/abs/1611.03947.

The subdirectory data_structure provides the source code for Graph data structure variants, as decribed below:
1. Sequential Graph data structure
2. Concurrent Coarse Lock Graph Data Structure
3. Concurrent Fine Lock Graph Data Structure without deletion of incoming edges
4. Concurrent Fine Lock Graph Data Structure with deletion of incoming edges

The subdirectory acyclicity provides the source code for Graph data structure variants which maintain acycylicity, as decribed below:
1. Sequential Graph data structure
2. Concurrent Coarse Lock Graph Data Structure
3. Concurrent Fine Lock Graph Data Structure without deletion of incoming edges (Cycle detection using collect)
4. Concurrent Fine Lock Graph Data Structure with deletion of incoming edges (Cycle detection using Reachability)
5. Concurrent Fine Lock Graph Data Structure without deletion of incoming edges (Cycle detection using collect)
6. Concurrent Fine Lock Graph Data Structure with deletion of incoming edges (Cycle detection using Reachability)

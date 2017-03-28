# ConcurrentGraphDS
This repository contains the implementation of a lock-based Concurrent Graph Data Structure.
The algorithm is described in technical report 'Maintaining Acyclicity of Concurrent Graph Objects' which can be accessed here: https://arxiv.org/abs/1611.03947.

Binary files can be found in bin directory.
Source files can be found in src directory.

The subdirectory src/data_structure provides the source code for Graph data structure variants, as decribed below:
1. Sequential Graph data structure
2. Concurrent Coarse Lock Graph Data Structure
3. Concurrent Fine Lock Graph Data Structure without deletion of incoming edges
4. Concurrent Fine Lock Graph Data Structure with deletion of incoming edges

The subdirectory src/acyclicity provides the source code for Graph data structure variants which maintain acycylicity, as decribed below:
1. Sequential Graph data structure
2. Concurrent Coarse Lock Graph Data Structure
3. Concurrent Fine Lock Graph Data Structure without deletion of incoming edges (Cycle detection using collect)
4. Concurrent Fine Lock Graph Data Structure with deletion of incoming edges (Cycle detection using Reachability)
5. Concurrent Fine Lock Graph Data Structure without deletion of incoming edges (Cycle detection using collect)
6. Concurrent Fine Lock Graph Data Structure with deletion of incoming edges (Cycle detection using Reachability)

If you have any questions, please contact: cs15mtech01004@iith.ac.in
